%{
#define YYSTYPE double // data type of yacc stack 
%}
%token NUMBER
%left  '+' '-' // left associative, same precedence
%left '*' '/' // left associative, higher precedence
%%
list:   // nothing
      | list '\n'
      | list expr '\n' { printf("\t%.8g\n", $2); }
expr:   NUMBER           { $$ = $1; }
      | expr '+' expr    { $$ = $1 + $3; }
      | expr '-' expr    { $$ = $1 - $3; } 
      | expr '*' expr    { $$ = $1 * $3; } 
      | expr '/' expr    { $$ = $1 / $3; } 
      | '(' expr ')'     { $$ = $2; }
      ;
%%

        // end of grammar

#include <stdio.h>
#include <ctype.h>

char *progname;
int lineno = 1;

int main(int argc, char *argv[]) {
    progname = argv[0]; // for error messages
    yyparse();
}

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
        scanf("%lf", &yylval);
        return NUMBER;
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
