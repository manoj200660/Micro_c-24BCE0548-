#include "lexer.h"

#include <ctype.h>
#include <string.h>

static const char *source;
static size_t current;
static int current_line;

static Token make_token(TokenType type, const char *lexeme, size_t length)
{
    Token token;

    token.type = type;
    token.line = current_line;

    if (length >= MAX_LEXEME_LENGTH)
    {
        length = MAX_LEXEME_LENGTH - 1;
    }

    memcpy(token.lexeme, lexeme, length);
    token.lexeme[length] = '\0';

    return token;
}

static Token error_token(const char *message)
{
    return make_token(TOKEN_ERROR, message, strlen(message));
}

static int is_at_end(void)
{
    return source == NULL || source[current] == '\0';
}

static char peek(void)
{
    if (is_at_end())
    {
        return '\0';
    }

    return source[current];
}

static char peek_next(void)
{
    if (is_at_end())
    {
        return '\0';
    }

    return source[current + 1];
}

static char advance_char(void)
{
    char c = peek();

    if (c != '\0')
    {
        current++;
    }

    return c;
}

static int match_char(char expected)
{
    if (peek() != expected)
    {
        return 0;
    }

    current++;
    return 1;
}

static Token identifier_or_keyword(void)
{
    size_t start = current;

    while (isalnum((unsigned char)peek()) || peek() == '_')
    {
        advance_char();
    }

    size_t length = current - start;
    const char *word = source + start;

    if (length == 3 && strncmp(word, "int", 3) == 0)
        return make_token(TOKEN_INT, word, length);

    if (length == 6 && strncmp(word, "string", 6) == 0)
        return make_token(TOKEN_STRING, word, length);

    if (length == 4 && strncmp(word, "func", 4) == 0)
        return make_token(TOKEN_FUNC, word, length);

    if (length == 2 && strncmp(word, "if", 2) == 0)
        return make_token(TOKEN_IF, word, length);

    if (length == 4 && strncmp(word, "else", 4) == 0)
        return make_token(TOKEN_ELSE, word, length);

    if (length == 5 && strncmp(word, "while", 5) == 0)
        return make_token(TOKEN_WHILE, word, length);

    if (length == 3 && strncmp(word, "for", 3) == 0)
        return make_token(TOKEN_FOR, word, length);

    if (length == 6 && strncmp(word, "return", 6) == 0)
        return make_token(TOKEN_RETURN, word, length);

    if (length == 5 && strncmp(word, "print", 5) == 0)
        return make_token(TOKEN_PRINT, word, length);

    if (length == 4 && strncmp(word, "read", 4) == 0)
        return make_token(TOKEN_READ, word, length);

    return make_token(TOKEN_IDENTIFIER, word, length);
}

static Token number(void)
{
    size_t start = current;

    while (isdigit((unsigned char)peek()))
    {
        advance_char();
    }

    return make_token(TOKEN_INT_LITERAL,
                      source + start,
                      current - start);
}

static Token string_literal(void)
{
    size_t start = current;

    while (!is_at_end() && peek() != '"')
    {
        if (peek() == '\n')
        {
            return error_token("Unterminated string literal");
        }

        advance_char();
    }

    if (is_at_end())
    {
        return error_token("Unterminated string literal");
    }

    size_t length = current - start;

    advance_char();

    return make_token(TOKEN_STRING_LITERAL,
                      source + start,
                      length);
}

void lexer_init(const char *input)
{
    source = input;
    current = 0;
    current_line = 1;
}

Token lexer_next_token(void)
{
    while (!is_at_end())
    {
        char c = peek();

        if (c == ' ' || c == '\r' || c == '\t')
        {
            advance_char();
            continue;
        }

        if (c == '\n')
        {
            advance_char();
            current_line++;
            continue;
        }

        if (c == '/' && peek_next() == '/')
        {
            while (!is_at_end() && peek() != '\n')
            {
                advance_char();
            }

            continue;
        }

        break;
    }

    if (is_at_end())
    {
        return make_token(TOKEN_EOF, "", 0);
    }

    char c = advance_char();

    if (isalpha((unsigned char)c) || c == '_')
    {
        current--;
        return identifier_or_keyword();
    }

    if (isdigit((unsigned char)c))
    {
        current--;
        return number();
    }

    if (c == '"')
    {
        return string_literal();
    }

    switch (c)
    {
        case '+':
            return make_token(TOKEN_PLUS, "+", 1);

        case '-':
            return make_token(TOKEN_MINUS, "-", 1);

        case '*':
            return make_token(TOKEN_MULTIPLY, "*", 1);

        case '/':
            return make_token(TOKEN_DIVIDE, "/", 1);

        case '%':
            return make_token(TOKEN_MODULO, "%", 1);

        case '=':
            if (match_char('='))
                return make_token(TOKEN_EQUAL, "==", 2);

            return make_token(TOKEN_ASSIGN, "=", 1);

        case '!':
            if (match_char('='))
                return make_token(TOKEN_NOT_EQUAL, "!=", 2);

            return make_token(TOKEN_NOT, "!", 1);

        case '<':
            if (match_char('='))
                return make_token(TOKEN_LESS_EQUAL, "<=", 2);

            return make_token(TOKEN_LESS, "<", 1);

        case '>':
            if (match_char('='))
                return make_token(TOKEN_GREATER_EQUAL, ">=", 2);

            return make_token(TOKEN_GREATER, ">", 1);

        case '&':
            if (match_char('&'))
                return make_token(TOKEN_AND, "&&", 2);

            return error_token("Unexpected '&'. Use '&&'.");

        case '|':
            if (match_char('|'))
                return make_token(TOKEN_OR, "||", 2);

            return error_token("Unexpected '|'. Use '||'.");

        case '(':
            return make_token(TOKEN_LEFT_PAREN, "(", 1);

        case ')':
            return make_token(TOKEN_RIGHT_PAREN, ")", 1);

        case '{':
            return make_token(TOKEN_LEFT_BRACE, "{", 1);

        case '}':
            return make_token(TOKEN_RIGHT_BRACE, "}", 1);

        case '[':
            return make_token(TOKEN_LEFT_BRACKET, "[", 1);

        case ']':
            return make_token(TOKEN_RIGHT_BRACKET, "]", 1);

        case ';':
            return make_token(TOKEN_SEMICOLON, ";", 1);

        case ',':
            return make_token(TOKEN_COMMA, ",", 1);

        default:
            return error_token("Unexpected character");
    }
}