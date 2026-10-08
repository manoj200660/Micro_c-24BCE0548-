#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *duplicate_string(const char *text)
{
    size_t length = strlen(text) + 1;
    char *result = malloc(length);

    if (result != NULL)
    {
        memcpy(result, text, length);
    }

    return result;
}

ASTNode *ast_new(ASTNodeType type, const char *text)
{
    ASTNode *node = calloc(1, sizeof(ASTNode));

    if (node == NULL)
    {
        return NULL;
    }

    node->type = type;
    node->text = duplicate_string(text != NULL ? text : "");

    if (node->text == NULL)
    {
        free(node);
        return NULL;
    }

    return node;
}

ASTNode *ast_binary(const char *op,
                    ASTNode *left,
                    ASTNode *right)
{
    ASTNode *node = ast_new(AST_BINARY, op);

    if (node != NULL)
    {
        node->left = left;
        node->right = right;
    }

    return node;
}

void ast_append(ASTNode **list, ASTNode *node)
{
    if (node == NULL)
    {
        return;
    }

    if (*list == NULL)
    {
        *list = node;
        return;
    }

    ASTNode *current = *list;

    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = node;
}

static const char *node_name(ASTNodeType type)
{
    switch (type)
    {
        case AST_PROGRAM: return "PROGRAM";
        case AST_FUNCTION: return "FUNCTION";
        case AST_BLOCK: return "BLOCK";
        case AST_DECL: return "DECLARATION";
        case AST_ASSIGN: return "ASSIGNMENT";
        case AST_PRINT: return "PRINT";
        case AST_RETURN: return "RETURN";
        case AST_INT: return "INT";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_BINARY: return "BINARY";
        default: return "NODE";
    }
}

static void print_node(const ASTNode *node, int indent)
{
    if (node == NULL)
    {
        return;
    }

    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }

    printf("%s", node_name(node->type));

    if (node->text != NULL && node->text[0] != '\0')
    {
        printf(" %s", node->text);
    }

    printf("\n");

    if (node->left != NULL)
    {
        print_node(node->left, indent + 1);
    }

    if (node->right != NULL)
    {
        print_node(node->right, indent + 1);
    }

    if (node->child != NULL)
    {
        print_node(node->child, indent + 1);
    }

    if (node->next != NULL)
    {
        print_node(node->next, indent);
    }
}

void ast_print(const ASTNode *node, int indent)
{
    print_node(node, indent);
}

void ast_free(ASTNode *node)
{
    while (node != NULL)
    {
        ASTNode *next = node->next;

        ast_free(node->left);
        ast_free(node->right);
        ast_free(node->child);

        free(node->text);
        free(node);

        node = next;
    }
}