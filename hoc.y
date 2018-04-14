%{
#include "hoc.h"
#define code2(c1, c2)      code(c1); code(c2)
#define code3(c1, c2, c3)  code(c1); code(c2); code(c3)
%}
%union {
    Symbol *sym;       // symbol table pointer
    Inst *inst;        // machine instruction
}
%token  <sym>           NUMBER VAR BLTIN UNDEF
%right                  '='
%left                   '+' '-'
%left                   '*' '/' '%'
%left                   UNARYPLUS
%left                   UNARYMINUS
%right                  '^'
%%
list:   //      nothing
        |       list '\n'
        |       list asgn '\n'  { code2(pop, STOP); return 1; }
        |       list expr '\n'  { code2(print, STOP); return 1; }
        |       list error '\n' { yyerrok; }
        ;
asgn:           VAR '=' expr { code3(varpush, (Inst)$1, assign); }
        ;
expr:           NUMBER           { code2(constpush, (Inst)$1); }
        |       VAR              { code3(varpush, (Inst)$1, eval); }
        |       asgn
        |       BLTIN '(' expr ')' { code2(bltin, (Inst)$1->u.ptr); }
        |       '('expr ')'
        |       expr '+' expr      { code(add); }
        |       expr '-' expr      { code(sub); }
        |       expr '*' expr      { code(mul); }
        |       expr '/' expr      { code(divide); }
        |       expr '%' expr      { code(mod); }
        |       expr '^' expr      { code(power); }
        |       '+' expr %prec UNARYPLUS { code(posit); }
        |       '-' expr %prec UNARYMINUS { code(negate); }
                ;
%%

#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>

char *progname;
int lineno = 1;
jmp_buf begin;
extern int init();

int yylex() {
    int c;

    while ((c = getchar()) == ' ' || c == '\t') {
        ;
    }

    if (c == EOF) {
        return 0;
    }

    if (c == '.' || isdigit(c)) { // number
        double d;
        ungetc(c, stdin);
        scanf("%lf", &d);
        yylval.sym = install("", NUMBER, d);
        return NUMBER;
    }

    if (isalpha(c)) {
        Symbol *s;
        char sbuf[100], *p = sbuf;
        do {
            *p++ = c;
        } while ((c = getchar()) != EOF && isalnum(c));
        ungetc(c, stdin);
        *p = '\0';
        if ((s = lookup(sbuf)) == 0) {
            s = install(sbuf, UNDEF, 0.0);
        }
        yylval.sym = s;
        return s->type == UNDEF ? VAR : s->type;
    }

    if (c == '\n') {
        lineno++;
    }

    return c;
}

int yyerror(char *s) {
    warning(s, NULL);
}

int warning(char *s, char *t) {
    fprintf(stderr, "%s: %s", progname, s);
    if (t) {
        fprintf(stderr, " %s", t);
    }
    fprintf(stderr, " near line %d\n", lineno);
}

int execerror(char *s, char *t) {
    warning(s, t);
    longjmp(begin, 0);
}

int fpecatch() {
    execerror("Floating point exception", NULL);
}

int main(int argc, char *argv[]) {
    progname = argv[0];
    init();
    setjmp(begin);
    signal(SIGFPE, fpecatch);
    for (initcode(); yyparse(); initcode()) {
        execute(prog);
    }
    return 0;
}
