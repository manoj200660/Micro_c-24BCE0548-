#include "token.h"

const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT_LITERAL: return "INT_LITERAL";
        case TOKEN_STRING_LITERAL: return "STRING_LITERAL";

        case TOKEN_INT: return "KEYWORD_INT";
        case TOKEN_STRING: return "KEYWORD_STRING";
        case TOKEN_FUNC: return "KEYWORD_FUNC";
        case TOKEN_IF: return "KEYWORD_IF";
        case TOKEN_ELSE: return "KEYWORD_ELSE";
        case TOKEN_WHILE: return "KEYWORD_WHILE";
        case TOKEN_FOR: return "KEYWORD_FOR";
        case TOKEN_RETURN: return "KEYWORD_RETURN";
        case TOKEN_PRINT: return "KEYWORD_PRINT";
        case TOKEN_READ: return "KEYWORD_READ";

        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_MULTIPLY: return "MULTIPLY";
        case TOKEN_DIVIDE: return "DIVIDE";
        case TOKEN_MODULO: return "MODULO";

        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";

        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_NOT: return "NOT";

        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";

        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";

        case TOKEN_ERROR: return "ERROR";

        default: return "UNKNOWN";
    }
}