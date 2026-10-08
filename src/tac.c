#include "tac.h"
#include "lexer.h"

#include <stdio.h>
#include <string.h>

static Token current_token;
static int temp_number;

static void next_token(void)
{
    current_token = lexer_next_token();
}

static int is_operator(TokenType type)
{
    return type == TOKEN_PLUS ||
           type == TOKEN_MINUS ||
           type == TOKEN_MULTIPLY ||
           type == TOKEN_DIVIDE;
}

void generate_tac(const char *source)
{
    char variable[128];
    char left[128];
    char right[128];
    char operator[8];
    char temp[32];

    temp_number = 0;

    lexer_init(source);
    next_token();

    printf("\n=== Three Address Code ===\n\n");

    while (current_token.type != TOKEN_EOF &&
           current_token.type != TOKEN_ERROR)
    {
        /* Skip function header */
        if (current_token.type == TOKEN_FUNC)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER)
            {
                next_token();
            }

            continue;
        }

        /* Skip declarations */
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

        /* Assignment */
        if (current_token.type == TOKEN_IDENTIFIER)
        {
            strcpy(variable, current_token.lexeme);

            next_token();

            if (current_token.type != TOKEN_ASSIGN)
            {
                continue;
            }

            next_token();

            /*
             * First operand
             */
            if (current_token.type != TOKEN_IDENTIFIER &&
                current_token.type != TOKEN_INT_LITERAL)
            {
                continue;
            }

            strcpy(left, current_token.lexeme);

            next_token();

            /*
             * Simple assignment
             * Example: x = 8;
             */
            if (current_token.type == TOKEN_SEMICOLON)
            {
                temp_number++;

                snprintf(
                    temp,
                    sizeof(temp),
                    "t%d",
                    temp_number
                );

                printf("%s = %s\n", temp, left);
                printf("%s = %s\n", variable, temp);

                next_token();

                continue;
            }

            /*
             * Binary expression
             * Example: y = x + 3;
             */
            if (is_operator(current_token.type))
            {
                strcpy(operator, current_token.lexeme);

                next_token();

                if (current_token.type == TOKEN_IDENTIFIER ||
                    current_token.type == TOKEN_INT_LITERAL)
                {
                    strcpy(right, current_token.lexeme);

                    next_token();

                    temp_number++;

                    snprintf(
                        temp,
                        sizeof(temp),
                        "t%d",
                        temp_number
                    );

                    printf(
                        "%s = %s %s %s\n",
                        temp,
                        left,
                        operator,
                        right
                    );

                    printf(
                        "%s = %s\n",
                        variable,
                        temp
                    );

                    if (current_token.type == TOKEN_SEMICOLON)
                    {
                        next_token();
                    }

                    continue;
                }
            }

            continue;
        }

        /* print */
        if (current_token.type == TOKEN_PRINT)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER ||
                current_token.type == TOKEN_INT_LITERAL)
            {
                printf(
                    "print %s\n",
                    current_token.lexeme
                );

                next_token();
            }

            if (current_token.type == TOKEN_SEMICOLON)
            {
                next_token();
            }

            continue;
        }

        /* return */
        if (current_token.type == TOKEN_RETURN)
        {
            next_token();

            if (current_token.type == TOKEN_IDENTIFIER ||
                current_token.type == TOKEN_INT_LITERAL)
            {
                printf(
                    "return %s\n",
                    current_token.lexeme
                );

                next_token();
            }

            if (current_token.type == TOKEN_SEMICOLON)
            {
                next_token();
            }

            continue;
        }

        next_token();
    }
}