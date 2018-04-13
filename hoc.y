%{
#include "hoc.h"
#include <math.h>
#include <stdio.h>

extern double Pow();
extern int execerror(char*, char*);
%}
%union {
    double val;        // actual value
    Symbol *sym;       // symbol table pointer
}
%token  <val>           NUMBER
%token  <sym>           VAR BLTIN UNDEF
%type   <val>           expr asgn
%right                  '='
%left                   '+' '-'
%left                   '*' '/' '%'
%left                   UNARYPLUS
%left                   UNARYMINUS
%right                  '^'
%%
list:   //      nothing
        |       list '\n'
        |       list asgn '\n'
        |       list expr '\n' { printf("\t%.8g\n", $2); }
        |       list error '\n' { yyerrok; }
        ;
asgn:           VAR '=' expr { $$ = $1->u.val = $3; $1->type = VAR; }
        ;
expr:           NUMBER
        |       VAR              {
                                      if ($1->type == UNDEF) {
                                          execerror("Undefined variable", $1->name);
                                      }
                                      $$ = $1->u.val;
                                 }
        |       asgn
        |       BLTIN '(' expr ')' { $$ = (*($1->u.ptr))($3); }
        |       expr '+' expr      { $$ = $1 + $3; }
        |       expr '-' expr      { $$ = $1 - $3; }
        |       expr '*' expr      { $$ = $1 * $3; }
        |       expr '/' expr      {
                                        if ($3 == 0.0) {
                                            execerror("division by zero", "");
                                        }
                                        $$ = $1 / $3;
                                   }
        // It would be nice to distinguish between integer and floating point
        // modulo operations in the future.
        |       expr '%' expr      { $$ = fmod($1, $3); }
        |       expr '^' expr      { $$ = Pow($1, $3); }
        |       '(' expr ')'       { $$ = $2; }
        |       '+' expr %prec UNARYPLUS { $$ = +$2; }
        |       '-' expr %prec UNARYMINUS { $$ = -$2; }
                ;
%%

        // end of grammar

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
        ungetc(c, stdin);
        scanf("%lf", &yylval.val);
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
    yyparse();
}
