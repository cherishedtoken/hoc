#include "hoc.h"
#include "y.tab.h"
#include <stdlib.h>
#include <string.h>

extern int execerror(char*, char*);
static Symbol *symlist = 0; // symbol table: linked list

char *emalloc(unsigned n) { // check return from malloc
    char *p = malloc(n);
    if (p == 0) {
        execerror("Out of memory", NULL);
    }
    return p;
}

Symbol *lookup(char *s) { // find s in symbol table
    for (Symbol *sp = symlist; sp != NULL; sp = sp->next) {
        if (strcmp(sp->name, s) == 0) {
            return sp;
        }
    }
    return 0; // not found
}

Symbol *install(char *s, int t, double d) { // install s in symbol table
    Symbol *sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = emalloc(strlen(s) + 1);
    strcpy(sp->name, s);
    sp->type = t;
    sp->u.val = d;
    sp->next = symlist; // put at front of list
    symlist = sp;
    return sp;
}

    
