#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "tac.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Could not open source file.\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);

    long size = ftell(file);

    rewind(file);

    if (size < 0)
    {
        fclose(file);
        return NULL;
    }

    char *buffer = malloc((size_t)size + 1);

    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, (size_t)size, file);

    buffer[bytes_read] = '\0';

    fclose(file);

    return buffer;
}

static void print_tokens(const char *source)
{
    Token token;

    lexer_init(source);

    printf("=== Lexical Analysis ===\n\n");
    printf("%-6s %-22s %s\n", "Line", "Token Type", "Lexeme");
    printf("------------------------------------------------------------\n");

    while (1)
    {
        token = lexer_next_token();

        printf("%-6d %-22s %s\n",
               token.line,
               token_type_name(token.type),
               token.lexeme);

        if (token.type == TOKEN_EOF)
        {
            break;
        }

        if (token.type == TOKEN_ERROR)
        {
            break;
        }
    }

    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    char *source = read_file(argv[1]);

    if (source == NULL)
    {
        return 1;
    }

    printf("=== Micro C Compiler ===\n\n");

    /* ---------------- Lexical Analysis ---------------- */

    print_tokens(source);

    /* Reset lexer before syntax analysis */
    lexer_init(source);

    /* ---------------- Syntax Analysis ---------------- */

    if (parse_program())
    {
        printf("=== Syntax Analysis ===\n");
        printf("Syntax Analysis: SUCCESS\n");
        printf("Program syntax is valid.\n\n");
    }
    else
    {
        printf("=== Syntax Analysis ===\n");
        printf("Syntax Analysis: FAILED\n");
        printf("%s\n", parser_error());

        free(source);
        return 1;
    }

    /* ---------------- Semantic Analysis ---------------- */

    if (semantic_analysis(source))
    {
        printf("=== Semantic Analysis ===\n");
        printf("Semantic Analysis: SUCCESS\n");
        printf("No semantic errors found.\n\n");
    }
    else
    {
        printf("=== Semantic Analysis ===\n");
        printf("Semantic Analysis: FAILED\n");
        printf("%s\n", semantic_error());

        free(source);
        return 1;
    }

    /* ---------------- Three Address Code ---------------- */

    generate_tac(source);

    free(source);

    return 0;
}