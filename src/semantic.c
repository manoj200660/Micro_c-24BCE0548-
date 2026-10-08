#include "semantic.h"
#include "lexer.h"

#include <stdio.h>
#include <string.h>

#define MAX_SYMBOLS 100
#define MAX_NAME 128

static char symbols[MAX_SYMBOLS][MAX_NAME];
static int symbol_count = 0;

static char error_message[256];

static Token current_token;

static void next_token(void)
{
    current_token = lexer_next_token();
}

static int find_symbol(const char *name)
{
    int i;

    for (i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbols[i], name) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int add_symbol(const char *name)
{
    if (find_symbol(name))
    {
        snprintf(
            error_message,
            sizeof(error_message),
            "Semantic Error: variable '%s' already declared",
            name
        );

        return 0;
    }

    if (symbol_count >= MAX_SYMBOLS)
    {
        snprintf(
            error_message,
            sizeof(error_message),
            "Semantic Error: symbol table is full"
        );

        return 0;
    }

    strcpy(symbols[symbol_count], name);
    symbol_count++;

    return 1;
}

static int is_declared(const char *name)
{
    return find_symbol(name);
}

int semantic_analysis(const char *source)
{
    char function_name[MAX_NAME];

    symbol_count = 0;
    error_message[0] = '\0';

    /*
     * First pass:
     * Collect all declared variables.
     */
    lexer_init(source);
    next_token();

    while (current_token.type != TOKEN_EOF &&
           current_token.type != TOKEN_ERROR)
    {
        /* Function name */
        if (current_token.type == TOKEN_FUNC)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER)
            {
                strcpy(function_name, current_token.lexeme);
                next_token();
            }

            continue;
        }

        /* int declaration */
        if (current_token.type == TOKEN_INT ||
            current_token.type == TOKEN_STRING)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER)
            {
                if (!add_symbol(current_token.lexeme))
                {
                    return 0;
                }

                next_token();
            }

            continue;
        }

        next_token();
    }

    /*
     * Second pass:
     * Check whether every used variable
     * has been declared.
     */
    lexer_init(source);
    next_token();

    while (current_token.type != TOKEN_EOF &&
           current_token.type != TOKEN_ERROR)
    {
        /* Skip function name */
        if (current_token.type == TOKEN_FUNC)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER)
            {
                next_token();
            }

            continue;
        }

        /* Skip declaration name */
        if (current_token.type == TOKEN_INT ||
            current_token.type == TOKEN_STRING)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER)
            {
                next_token();
            }

            continue;
        }

        /* Check variable usage */
        if (current_token.type == TOKEN_IDENTIFIER)
        {
            if (!is_declared(current_token.lexeme))
            {
                snprintf(
                    error_message,
                    sizeof(error_message),
                    "Semantic Error: variable '%s' is not declared",
                    current_token.lexeme
                );

                return 0;
            }
        }

        next_token();
    }

    return 1;
}

const char *semantic_error(void)
{
    return error_message;
}