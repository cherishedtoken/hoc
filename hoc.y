%{
double mem[26]; // memory for variables 'a'..'z'
%}
%union {               // stack type
    double val;        // actual value
    int index;         // index into mem[]
}
%token  <val>           NUMBER
%token  <index>         VAR
%type   <val>           expr
%right                  '='
%left                   '+' '-'
%left                   UNARYPLUS
%left                   '*' '/' '%'
%left                   UNARYMINUS
%%
list:   //      nothing
        |       list '\n'
        |       list expr '\n' { printf("\t%.8g\n", $2); }
        |       list error '\n' { yyerrok; }
expr:           NUMBER
        |       VAR              { $$ = mem[$1]; }
        |       VAR '=' expr     { $$ = mem[$1] = $3; }
        |       expr '+' expr    { $$ = $1 + $3; }
        |       expr '-' expr    { $$ = $1 - $3; }
        |       expr '*' expr    { $$ = $1 * $3; }
        |       expr '/' expr    {
         if ($3 == 0.0) {
             execerror("division by zero", "");
         }
         $$ = $1 / $3;
         }
        // It would be nice to distinguish between integer and floating point modulo operations
        |       expr '%' expr      { $$ = fmod($1, $3); } 
        |       '(' expr ')'     { $$ = $2; }
        |       '-' expr %prec UNARYMINUS { $$ = -$2; }
        |       '+' expr %prec UNARYPLUS { $$ = +$2; }
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

    if (islower(c)) {
        yylval.index = c - 'a'; // ASCII only
        return VAR;
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
    setjmp(begin);
    signal(SIGFPE, fpecatch);
    yyparse();
}
