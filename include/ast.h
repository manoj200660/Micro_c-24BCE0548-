#ifndef AST_H
#define AST_H

#include "token.h"

typedef enum
{
    AST_PROGRAM,
    AST_FUNCTION,
    AST_BLOCK,
    AST_DECL,
    AST_ASSIGN,
    AST_PRINT,
    AST_RETURN,
    AST_INT,
    AST_IDENTIFIER,
    AST_BINARY
} ASTNodeType;

typedef struct ASTNode ASTNode;

struct ASTNode
{
    ASTNodeType type;
    char *text;

    ASTNode *left;
    ASTNode *right;
    ASTNode *child;
    ASTNode *next;
};

ASTNode *ast_new(ASTNodeType type, const char *text);

ASTNode *ast_binary(const char *op,
                    ASTNode *left,
                    ASTNode *right);

void ast_append(ASTNode **list, ASTNode *node);

void ast_print(const ASTNode *node, int indent);

void ast_free(ASTNode *node);

#endif