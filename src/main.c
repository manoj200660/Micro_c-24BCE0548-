#include "lexer.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        perror("Error opening source file");
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

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    char *source_code = read_file(argv[1]);

    if (source_code == NULL)
    {
        return 1;
    }

    lexer_init(source_code);

    printf("=== Micro C Compiler - Lexical Analysis ===\n\n");
    printf("%-6s %-22s %s\n", "Line", "Token Type", "Lexeme");
    printf("------------------------------------------------------------\n");

    while (1)
    {
        Token token = lexer_next_token();

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
            free(source_code);
            return 1;
        }
    }

    free(source_code);

    return 0;
}