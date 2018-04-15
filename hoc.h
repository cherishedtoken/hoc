typedef struct Symbol { // symbol table entry
    char *name;
    short type;
    union {
        double val;         // VAR
        double (*ptr)();    // BLTIN
        int    (*defn)();   // FUNCTION, PROCEDURE
        char   *str;        // STRING
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

extern Inst prog[], *progp, *code(Inst);
extern void eval(), add(), sub(), mul(), divide(), mod(), negate(), posit(),
    power(), assign(), bltin(), varpush(), constpush(), print(), initcode(),
    execute(Inst*), prstr(), prexpr(), gt(), lt(), eq(), ge(), le(), ne(),
    and(), or(), not(), ifcode(), whilecode(), define(Symbol *), call(), ret(),
    procret(), funcret(), arg(), argassign(), varread();
extern double *getarg();
