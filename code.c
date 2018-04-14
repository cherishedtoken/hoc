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

void initcode() {
    stackp = stack;
    progp = prog;
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
    for (pc = p; *pc != STOP; ) {
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

