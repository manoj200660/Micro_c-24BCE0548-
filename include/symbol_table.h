#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define MAX_SYMBOLS 128

typedef struct
{
    char name[128];
    char type[32];
    char scope[128];
} Symbol;

typedef struct
{
    Symbol items[MAX_SYMBOLS];
    int count;
} SymbolTable;

void symtab_init(SymbolTable *table);

int symtab_insert(SymbolTable *table,
                  const char *name,
                  const char *type,
                  const char *scope);

const Symbol *symtab_lookup(const SymbolTable *table,
                            const char *name);

void symtab_print(const SymbolTable *table);

#endif