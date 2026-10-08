#include "parser.h"
#include "lexer.h"

#include <stdio.h>

static Token current;
static char error_message[200];

static void next_token(void)
{
    current = lexer_next_token();
}

static int expect(TokenType type, const char *name)
{
    if (current.type == type)
    {
        next_token();
        return 1;
    }

    snprintf(
        error_message,
        sizeof(error_message),
        "Syntax Error at line %d: expected %s, found %s",
        current.line,
        name,
        token_type_name(current.type)
    );

    return 0;
}

int parse_program(void)
{
    error_message[0] = '\0';

    next_token();

    /* func */
    if (!expect(TOKEN_FUNC, "func"))
        return 0;

    /* function name */
    if (!expect(TOKEN_IDENTIFIER, "function name"))
        return 0;

    /* ( */
    if (!expect(TOKEN_LEFT_PAREN, "("))
        return 0;

    /* ) */
    if (!expect(TOKEN_RIGHT_PAREN, ")"))
        return 0;

    /* { */
    if (!expect(TOKEN_LEFT_BRACE, "{"))
        return 0;

    /* Read statements until } */
    while (current.type != TOKEN_RIGHT_BRACE &&
           current.type != TOKEN_EOF)
    {
        /* int x; */
        if (current.type == TOKEN_INT)
        {
            next_token();

            if (!expect(TOKEN_IDENTIFIER, "identifier"))
                return 0;

            if (!expect(TOKEN_SEMICOLON, ";"))
                return 0;

            continue;
        }

        /* x = ... ; */
        if (current.type == TOKEN_IDENTIFIER)
        {
            next_token();

            if (!expect(TOKEN_ASSIGN, "="))
                return 0;

            /* Skip expression until ; */
            while (current.type != TOKEN_SEMICOLON &&
                   current.type != TOKEN_EOF)
            {
                next_token();
            }

            if (!expect(TOKEN_SEMICOLON, ";"))
                return 0;

            continue;
        }

        /* print x; */
        if (current.type == TOKEN_PRINT)
        {
            next_token();

            if (!expect(TOKEN_IDENTIFIER, "identifier"))
                return 0;

            if (!expect(TOKEN_SEMICOLON, ";"))
                return 0;

            continue;
        }

        /* return x; */
        if (current.type == TOKEN_RETURN)
        {
            next_token();

            if (!expect(TOKEN_IDENTIFIER, "identifier"))
                return 0;

            if (!expect(TOKEN_SEMICOLON, ";"))
                return 0;

            continue;
        }

        snprintf(
            error_message,
            sizeof(error_message),
            "Syntax Error at line %d: unexpected token %s",
            current.line,
            token_type_name(current.type)
        );

        return 0;
    }

    /* } */
    if (!expect(TOKEN_RIGHT_BRACE, "}"))
        return 0;

    /* End of file */
    if (current.type != TOKEN_EOF)
    {
        snprintf(
            error_message,
            sizeof(error_message),
            "Syntax Error: unexpected tokens after program"
        );

        return 0;
    }

    return 1;
}

const char *parser_error(void)
{
    return error_message;
}