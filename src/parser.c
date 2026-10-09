
#include "parser.h"
#include "lexer.h"

#include <stdio.h>
#include <string.h>

static Token current_token;
static char error_message[256];
static int failed;

static void next_token(void)
{
    current_token = lexer_next_token();
}

static int match(TokenType type)
{
    if (current_token.type == type)
    {
        next_token();
        return 1;
    }

    return 0;
}

static void syntax_error(const char *expected)
{
    if (!failed)
    {
        snprintf(
            error_message,
            sizeof(error_message),
            "Syntax Error at line %d: expected %s, found %s",
            current_token.line,
            expected,
            token_type_name(current_token.type)
        );
    }

    failed = 1;
}

static int expect(TokenType type, const char *description)
{
    if (match(type))
        return 1;

    syntax_error(description);
    return 0;
}

static int expression(void);
static int statement(void);
static int block(void);

/* Primary expressions: identifiers, literals, arrays and parentheses. */
static int primary(void)
{
    if (failed)
        return 0;

    if (match(TOKEN_INT_LITERAL) ||
        match(TOKEN_STRING_LITERAL))
        return 1;

    if (match(TOKEN_LEFT_PAREN))
    {
        if (!expression())
            return 0;

        return expect(TOKEN_RIGHT_PAREN, "')'");
    }

    if (match(TOKEN_IDENTIFIER))
    {
        /* Array indexing and function calls. */
        for (;;)
        {
            if (match(TOKEN_LEFT_BRACKET))
            {
                if (!expression())
                    return 0;

                if (!expect(TOKEN_RIGHT_BRACKET, "']'"))
                    return 0;
            }
            else if (match(TOKEN_LEFT_PAREN))
            {
                if (current_token.type != TOKEN_RIGHT_PAREN)
                {
                    do
                    {
                        if (!expression())
                            return 0;
                    }
                    while (match(TOKEN_COMMA));
                }

                if (!expect(TOKEN_RIGHT_PAREN, "')'"))
                    return 0;
            }
            else
            {
                break;
            }
        }

        return 1;
    }

    syntax_error("an expression");
    return 0;
}

static int unary(void)
{
    if (match(TOKEN_NOT) || match(TOKEN_MINUS) ||
        match(TOKEN_PLUS))
        return unary();

    return primary();
}

static int multiplicative(void)
{
    if (!unary())
        return 0;

    while (current_token.type == TOKEN_MULTIPLY ||
           current_token.type == TOKEN_DIVIDE ||
           current_token.type == TOKEN_MODULO)
    {
        next_token();

        if (!unary())
            return 0;
    }

    return 1;
}

static int additive(void)
{
    if (!multiplicative())
        return 0;

    while (current_token.type == TOKEN_PLUS ||
           current_token.type == TOKEN_MINUS)
    {
        next_token();

        if (!multiplicative())
            return 0;
    }

    return 1;
}

static int relational(void)
{
    if (!additive())
        return 0;

    while (current_token.type == TOKEN_LESS ||
           current_token.type == TOKEN_LESS_EQUAL ||
           current_token.type == TOKEN_GREATER ||
           current_token.type == TOKEN_GREATER_EQUAL)
    {
        next_token();

        if (!additive())
            return 0;
    }

    return 1;
}

static int equality(void)
{
    if (!relational())
        return 0;

    while (current_token.type == TOKEN_EQUAL ||
           current_token.type == TOKEN_NOT_EQUAL)
    {
        next_token();

        if (!relational())
            return 0;
    }

    return 1;
}

static int logical_and(void)
{
    if (!equality())
        return 0;

    while (match(TOKEN_AND))
    {
        if (!equality())
            return 0;
    }

    return 1;
}

static int logical_or(void)
{
    if (!logical_and())
        return 0;

    while (match(TOKEN_OR))
    {
        if (!logical_and())
            return 0;
    }

    return 1;
}

static int expression(void)
{
    return logical_or();
}

/* Declarations such as int x; and int arr[5]; */
static int declaration(void)
{
    if (!match(TOKEN_INT) && !match(TOKEN_STRING))
    {
        syntax_error("'int' or 'string'");
        return 0;
    }

    if (!expect(TOKEN_IDENTIFIER, "an identifier"))
        return 0;

    if (match(TOKEN_LEFT_BRACKET))
    {
        if (!expect(TOKEN_INT_LITERAL, "an array size"))
            return 0;

        if (!expect(TOKEN_RIGHT_BRACKET, "']'"))
            return 0;
    }

    return expect(TOKEN_SEMICOLON, "';'");
}

/* Assignment targets can be variables or array elements. */
static int assignment(void)
{
    if (!expect(TOKEN_IDENTIFIER, "an assignment target"))
        return 0;

    while (match(TOKEN_LEFT_BRACKET))
    {
        if (!expression())
            return 0;

        if (!expect(TOKEN_RIGHT_BRACKET, "']'"))
            return 0;
    }

    if (!expect(TOKEN_ASSIGN, "'='"))
        return 0;

    if (!expression())
        return 0;

    return 1;
}

static int simple_statement(void)
{
    if (current_token.type == TOKEN_IDENTIFIER)
    {
        if (!assignment())
            return 0;

        return expect(TOKEN_SEMICOLON, "';'");
    }

    if (match(TOKEN_PRINT) || match(TOKEN_RETURN))
    {
        if (current_token.type != TOKEN_SEMICOLON)
        {
            if (!expression())
                return 0;
        }

        return expect(TOKEN_SEMICOLON, "';'");
    }

    if (match(TOKEN_READ))
    {
        if (!expect(TOKEN_IDENTIFIER, "an identifier"))
            return 0;

        return expect(TOKEN_SEMICOLON, "';'");
    }

    syntax_error("a statement");
    return 0;
}

static int block(void)
{
    if (!expect(TOKEN_LEFT_BRACE, "'{'"))
        return 0;

    while (!failed &&
           current_token.type != TOKEN_RIGHT_BRACE &&
           current_token.type != TOKEN_EOF)
    {
        if (!statement())
            return 0;
    }

    return expect(TOKEN_RIGHT_BRACE, "'}'");
}

static int statement(void)
{
    if (failed)
        return 0;

    if (current_token.type == TOKEN_INT ||
        current_token.type == TOKEN_STRING)
        return declaration();

    if (current_token.type == TOKEN_LEFT_BRACE)
        return block();

    if (match(TOKEN_IF))
    {
        if (!expect(TOKEN_LEFT_PAREN, "'('"))
            return 0;

        if (!expression())
            return 0;

        if (!expect(TOKEN_RIGHT_PAREN, "')'"))
            return 0;

        if (!statement())
            return 0;

        if (match(TOKEN_ELSE))
        {
            if (!statement())
                return 0;
        }

        return 1;
    }

    if (match(TOKEN_WHILE))
    {
        if (!expect(TOKEN_LEFT_PAREN, "'('"))
            return 0;

        if (!expression())
            return 0;

        if (!expect(TOKEN_RIGHT_PAREN, "')'"))
            return 0;

        return statement();
    }

    if (match(TOKEN_FOR))
    {
        if (!expect(TOKEN_LEFT_PAREN, "'('"))
            return 0;

        /* Initialization clause. */
        if (current_token.type != TOKEN_SEMICOLON)
        {
            if (!assignment())
                return 0;
        }

        if (!expect(TOKEN_SEMICOLON, "';'"))
            return 0;

        /* Loop condition. */
        if (current_token.type != TOKEN_SEMICOLON)
        {
            if (!expression())
                return 0;
        }

        if (!expect(TOKEN_SEMICOLON, "';'"))
            return 0;

        /* Update clause. */
        if (current_token.type != TOKEN_RIGHT_PAREN)
        {
            if (!assignment())
                return 0;
        }

        if (!expect(TOKEN_RIGHT_PAREN, "')'"))
            return 0;

        return statement();
    }

    return simple_statement();
}

static int function_declaration(void)
{
    if (!expect(TOKEN_FUNC, "'func'"))
        return 0;

    if (!expect(TOKEN_IDENTIFIER, "a function name"))
        return 0;

    if (!expect(TOKEN_LEFT_PAREN, "'('"))
        return 0;

    /* Optional parameters: int a, string b */
    if (current_token.type != TOKEN_RIGHT_PAREN)
    {
        do
        {
            if (!match(TOKEN_INT) && !match(TOKEN_STRING))
            {
                syntax_error("a parameter type");
                return 0;
            }

            if (!expect(TOKEN_IDENTIFIER, "a parameter name"))
                return 0;
        }
        while (match(TOKEN_COMMA));
    }

    if (!expect(TOKEN_RIGHT_PAREN, "')'"))
        return 0;

    return block();
}

int parse_program(void)
{
    failed = 0;
    error_message[0] = '\0';

    next_token();

    while (!failed && current_token.type != TOKEN_EOF)
    {
        if (!function_declaration())
            return 0;
    }

    return !failed;
}

const char *parser_error(void)
{
    return error_message;
}