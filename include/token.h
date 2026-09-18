#ifndef TOKEN_H
#define TOKEN_H

#define MAX_LEXEME_LENGTH 128

typedef enum {
    TOKEN_EOF,

    /* Identifiers and literals */
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_STRING_LITERAL,

    /* Keywords */
    TOKEN_INT,
    TOKEN_STRING,
    TOKEN_FUNC,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_READ,

    /* Arithmetic operators */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,

    /* Assignment and comparison */
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    /* Logical operators */
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    /* Delimiters */
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,

    /* Error */
    TOKEN_ERROR

} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LENGTH];
    int line;
} Token;

const char *token_type_name(TokenType type);

#endif