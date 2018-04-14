typedef struct Symbol { // symbol table entry
    char *name;
    short type;             // VAR, BLTIN, UNDEF
    union {
        double val;         // if VAR
        double (*ptr)();    // if BLTIN
    } u;
    struct Symbol *next;
} Symbol;

Symbol *install(), *lookup();

typedef union Datum { // interpreter stack data
    double val;
    Symbol *sym;
} Datum;
extern Datum pop();

typedef int (*Inst)(); // machine instruction
#define STOP (Inst) 0

extern Inst prog[];
extern Inst *code(Inst);
extern void eval(), add(), sub(), mul(), divide(), mod(), negate(), posit(), power();
extern void assign(), bltin(), varpush(), constpush(), print(), initcode(), execute(Inst*);
