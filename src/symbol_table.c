#include "symbol_table.h"

#include <stdio.h>
#include <string.h>

void symtab_init(SymbolTable *table)
{
    table->count = 0;
}

int symtab_insert(SymbolTable *table,
                  const char *name,
                  const char *type,
                  const char *scope)
{
    if (symtab_lookup(table, name) != NULL)
    {
        return 0;
    }

    if (table->count >= MAX_SYMBOLS)
    {
        return 0;
    }

    strncpy(table->items[table->count].name,
            name,
            127);

    table->items[table->count].name[127] = '\0';

    strncpy(table->items[table->count].type,
            type,
            31);

    table->items[table->count].type[31] = '\0';

    strncpy(table->items[table->count].scope,
            scope,
            127);

    table->items[table->count].scope[127] = '\0';

    table->count++;

    return 1;
}

const Symbol *symtab_lookup(const SymbolTable *table,
                            const char *name)
{
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->items[i].name, name) == 0)
        {
            return &table->items[i];
        }
    }

    return NULL;
}

void symtab_print(const SymbolTable *table)
{
    printf("Name        Type        Scope\n");
    printf("--------------------------------\n");

    for (int i = 0; i < table->count; i++)
    {
        printf("%-11s %-11s %s\n",
               table->items[i].name,
               table->items[i].type,
               table->items[i].scope);
    }
}