#include "hoc.h"
#include "y.tab.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

extern int execerror(char*, char*);

#define NSTACK 256
static Datum stack[NSTACK];
static Datum *stackp; // next free spot on the stack

#define NPROG 2000
Inst prog[NPROG]; // the machine
Inst *progp; // next free spot for code generation
Inst *pc; // program counter during execution
Inst *progbase = prog; // start of current subprogram
int returning; // 1 if return stmt seen

typedef struct Frame { // proc/func call stack frame
    Symbol *sp; // symbol table entry
    Inst *retpc; // where to resume after return
    Datum *argn; // n-th argument on stack
    int nargs; // number of args
} Frame;

#define NFRAME 100
Frame frame[NFRAME];
Frame *fp; // frame pointer

void initcode() {
    progp = progbase;
    stackp = stack;
    fp = frame;
    returning = 0;
}

void push(Datum d) {
    if (stackp >= &stack[NSTACK]) {
        execerror("Stack overflow", NULL);
    }
    *stackp++ = d;
}

Datum pop() {
    if (stackp <= stack) {
        execerror("Stack underflow", NULL);
    }
    return *--stackp;
}

Inst *code(Inst f) { // Install one instruction or operand
    Inst *oprogp = progp;
    if (progp >= &prog[NPROG]) {
        execerror("Program too big", NULL);
    }
    *progp++ = f;
    return oprogp;
}

void execute(Inst *p) {
    for (pc = p; *pc != STOP && !returning; ) {
        (*(*pc++))();
    }
}

void constpush() {
    Datum d;
    d.val = ((Symbol *)*pc++)->u.val;
    push(d);
}

void varpush() {
    Datum d;
    d.sym = (Symbol *)(*pc++);
    push(d);
}

void add() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val += d2.val;
    push(d1);
}

void sub() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val -= d2.val;
    push(d1);
}

void mul() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val *= d2.val;
    push(d1);
}

void divide() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val /= d2.val;
    push(d1);
}

void power() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    double result = pow(d1.val, d2.val);
    d1.val = result;
    push(d1);
}

void mod() {
    // Make this better at some point
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    float result = fmod(d1.val, d2.val);
    d1.val = result;
    push(d1);
}

void negate() {
    Datum d = pop();
    d.val = -d.val;
    push(d);
}

void posit() {
    Datum d = pop();
    d.val = +d.val;
    push(d);
}

void eval() {
    Datum d = pop();
    if (d.sym->type == UNDEF) {
        execerror("Undefined variable", d.sym->name);
    }
    d.val = d.sym->u.val;
    push(d);
}

void assign() {
    Datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF) {
        execerror("Assignment to non-variable", d1.sym->name);
    }
    d1.sym->u.val = d2.val;
    d1.sym->type = VAR;
    push(d2);
}

void print() {
    Datum d = pop();
    printf("\t%.8g\n", d.val);
}

void bltin() {
    Datum d = pop();
    d.val = (*(double (*)())(*pc++))(d.val);
    push(d);
}

void gt() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val > d2.val);
    push(d1);
}

void lt() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val < d2.val);
    push(d1);
}

void eq() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val == d2.val);
    push(d1);
}

void ge() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val >= d2.val);
    push(d1);
}

void le() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val <= d2.val);
    push(d1);
}

void ne() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != d2.val);
    push(d1);
}

void and() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val && d2.val);
    push(d1);
}

void or() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val || d2.val);
    push(d1);
}

void not() {
    Datum d = pop();
    d.val = (double)(d.val == 0.0);
    push(d);
}

void whilecode() {
    Inst *savepc = pc; // loop body
    
    execute(savepc + 2); // condition
    Datum d = pop();
    while (d.val) {
        execute(*((Inst **)(savepc))); // body
        if (returning) {
            break;
        }
        execute(savepc + 2);
        d = pop();
    }

    if (!returning) {
        pc = *((Inst **)(savepc + 1)); // next statement
    }
}

void ifcode() {
    Inst *savepc = pc; // then part
    
    execute(savepc + 3); // condition
    Datum d = pop();
    if (d.val) {
        execute(*((Inst **)(savepc)));
    } else if (*((Inst **)(savepc + 1))) { // else part?
        execute(*((Inst **)(savepc + 1)));
    }
    
    if (!returning) {
        pc = *((Inst **)(savepc + 2)); // next statement
    }
}

void prstr() { // print string value
    printf("%s", (char *) *pc++);
}

void prexpr() { // print numeric value
    Datum d = pop();
    printf("%.8g\n", d.val);
}

void define(Symbol *sp) { // put func/proc in symbol table
    sp->u.defn = (Inst)progbase; // start of code
    progbase = progp; // next code starts here
}

void call() {
    Symbol *sp = (Symbol *)pc[0]; // symtable entry for function
    if (fp++ >= &frame[NFRAME - 1]) {
        execerror(sp->name, "call nested too deeply");
    }
    fp->sp = sp;
    fp->nargs = (int)pc[1];
    fp->retpc = pc + 2;
    fp->argn = stackp - 1; // last arg
    execute(sp->u.defn);
    returning = 0;
}

void ret() { // common return from func or proc
    for (int i = 0; i < fp->nargs; i++) {
        pop(); // pop args
    }
    pc = (Inst *)fp->retpc;
    --fp;
    returning = 1;
}

void procret() {
    if (fp->sp->type == FUNCTION) {
        execerror(fp->sp->name, "(func) returns no value");
    }
    ret();
}

void funcret() {
    if (fp->sp->type == PROCEDURE) {
        execerror(fp->sp->name, "(proc) returns value");
    }
    Datum d = pop(); // preserve function return value
    ret();
    push(d);
}

double *getarg() { // return pointer to argument
    int nargs = (int) *pc++;
    if (nargs > fp->nargs) {
        execerror(fp->sp->name, "not enough arguments");
    }
    return &fp->argn[nargs - fp->nargs].val;
}

void arg() { // push arg onto stack
    Datum d;
    d.val = *getarg();
    push(d);
}

void argassign() { // store top of stack in arg
    Datum d = pop();
    push(d); // leave val on stack
    *getarg() = d.val;
}

void varread() { // read into variable
    Datum d;
    extern FILE *fin;
    Symbol *var = (Symbol *) *pc++;
    
 Again:
    switch(fscanf(fin, "%lf", &var->u.val)) {
    case EOF:
        if (moreinput()) {
            goto Again; // gah
        }
        d.val = var->u.val = 0.0;
        break;
    case 0:
        execerror("non-number read into", var->name);
        break;
    default:
        d.val = 1.0;
        break;
    }
    var->type = VAR;
    push(d);
}
