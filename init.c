#include "hoc.h"
#include "y.tab.h"
#include <math.h>

extern double Log(), Log10(), Exp(), Sqrt(), Integer();

static struct {
    char *name;
    double cval;
} consts[] = {
    "PI",      3.14159265358979323846,
    "E",       2.71828182845904523536,
    "GAMMA",   0.57721566490153286060, // Euler
    "DEG",     57.29577951308232087680, // deg/radian
    "PHI",     1.61803398874989484820,
    0,         0
};

static struct {
    char *name;
    double (*func)();
} builtins[] = {
    "sin", sin,
    "cos", cos,
    "atan", atan,
    "log", Log,        // Checks argument
    "log10", Log10,    // Checks argument
    "exp", Exp,        // Checks argument
    "sqrt", Sqrt,      // Checks argument
    "int", Integer,
    "abs", fabs,
    0, 0
};

static struct {
    char *name;
    int kval;
} keywords[] = {
    "proc",      PROC,
    "func",      FUNC,
    "return",    RETURN,
    "if",        IF,
    "else",      ELSE,
    "while",     WHILE,
    "print",     PRINT,
    "read",      READ,
    0,           0,
};

int init() {
    for (int i = 0; consts[i].name; i++) {
        install(consts[i].name, VAR, consts[i].cval);
    }
    
    for (int j = 0; builtins[j].name; j++) {
        Symbol *s = install(builtins[j].name, BLTIN, 0.0);
        s->u.ptr = builtins[j].func;
    }

    for (int k = 0; keywords[k].name; k++) {
        install(keywords[k].name, keywords[k].kval, 0.0);
    }
}
