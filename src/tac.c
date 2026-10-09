
#include "tac.h"
#include "lexer.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAX_OUTPUT 65536
#define MAX_TEXT 128

static Token token;
static char output[MAX_OUTPUT];
static size_t output_length;
static int temp_count;
static int label_count;
static int failed;

typedef struct {
    char text[MAX_TEXT];
} Value;

static void next_token(void)
{
    token = lexer_next_token();

    if (token.type == TOKEN_ERROR)
        failed = 1;
}

static int match(TokenType type)
{
    if (token.type != type)
        return 0;

    next_token();
    return 1;
}

static int expect(TokenType type)
{
    if (token.type != type) {
        failed = 1;
        return 0;
    }

    next_token();
    return 1;
}

static void emit(const char *format, ...)
{
    char line[512];
    va_list args;
    int n;

    va_start(args, format);
    n = vsnprintf(line, sizeof(line), format, args);
    va_end(args);

    if (n < 0 || output_length + (size_t)n + 1 >= MAX_OUTPUT) {
        failed = 1;
        return;
    }

    memcpy(output + output_length, line, (size_t)n);
    output_length += (size_t)n;
    output[output_length] = '\0';
}

static Value value_of(const char *text)
{
    Value v;
    snprintf(v.text, sizeof(v.text), "%s", text);
    return v;
}

static Value new_temp(void)
{
    char name[MAX_TEXT];
    snprintf(name, sizeof(name), "t%d", ++temp_count);
    return value_of(name);
}

static Value new_label(void)
{
    char name[MAX_TEXT];
    snprintf(name, sizeof(name), "L%d", ++label_count);
    return value_of(name);
}



static Value expression(void);

static Value primary(void)
{
    Value result = value_of("0");

    if (failed)
        return result;

    if (token.type == TOKEN_INT_LITERAL ||
        token.type == TOKEN_STRING_LITERAL) {
        result = value_of(token.lexeme);
        next_token();
        return result;
    }

    if (match(TOKEN_LEFT_PAREN)) {
        result = expression();
        expect(TOKEN_RIGHT_PAREN);
        return result;
    }

    if (token.type == TOKEN_IDENTIFIER) {
        char name[MAX_TEXT];

        snprintf(name, sizeof(name), "%s", token.lexeme);
        next_token();

        if (match(TOKEN_LEFT_BRACKET)) {
            Value index = expression();
            Value temp = new_temp();

            expect(TOKEN_RIGHT_BRACKET);
            emit("%s = %s[%s]\n", temp.text, name, index.text);

            return temp;
        }

        if (match(TOKEN_LEFT_PAREN)) {
            /* Basic function-call TAC. */
            while (!failed && token.type != TOKEN_RIGHT_PAREN &&
                   token.type != TOKEN_EOF) {
                Value arg = expression();
                emit("param %s\n", arg.text);

                if (!match(TOKEN_COMMA))
                    break;
            }

            expect(TOKEN_RIGHT_PAREN);

            result = new_temp();
            emit("%s = call %s\n", result.text, name);
            return result;
        }

        return value_of(name);
    }

    failed = 1;
    return result;
}

static Value unary(void)
{
    if (token.type == TOKEN_MINUS ||
        token.type == TOKEN_NOT ||
        token.type == TOKEN_PLUS) {
        char op[MAX_TEXT];

        snprintf(op, sizeof(op), "%s", token.lexeme);
        next_token();

        Value operand = unary();

        if (strcmp(op, "+") == 0)
            return operand;

        Value temp = new_temp();
        emit("%s = %s%s\n", temp.text, op, operand.text);

        return temp;
    }

    return primary();
}

static Value binary_level(Value (*lower)(void),
                          const TokenType *operators,
                          size_t operator_count)
{
    Value left = lower();

    while (!failed) {
        int found = 0;

        for (size_t i = 0; i < operator_count; i++) {
            if (token.type == operators[i]) {
                found = 1;
                break;
            }
        }

        if (!found)
            break;

        char op[MAX_TEXT];
        snprintf(op, sizeof(op), "%s", token.lexeme);
        next_token();

        Value right = lower();
        Value temp = new_temp();

        emit("%s = %s %s %s\n",
             temp.text, left.text, op, right.text);

        left = temp;
    }

    return left;
}

static Value multiplicative(void)
{
    const TokenType ops[] = {
        TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_MODULO
    };

    return binary_level(unary, ops, sizeof(ops) / sizeof(ops[0]));
}

static Value additive(void)
{
    const TokenType ops[] = {TOKEN_PLUS, TOKEN_MINUS};

    return binary_level(multiplicative, ops,
                        sizeof(ops) / sizeof(ops[0]));
}

static Value relational(void)
{
    const TokenType ops[] = {
        TOKEN_LESS, TOKEN_LESS_EQUAL,
        TOKEN_GREATER, TOKEN_GREATER_EQUAL
    };

    return binary_level(additive, ops, sizeof(ops) / sizeof(ops[0]));
}

static Value equality(void)
{
    const TokenType ops[] = {TOKEN_EQUAL, TOKEN_NOT_EQUAL};

    return binary_level(relational, ops, sizeof(ops) / sizeof(ops[0]));
}

static Value logical_and(void)
{
    const TokenType ops[] = {TOKEN_AND};

    return binary_level(equality, ops, sizeof(ops) / sizeof(ops[0]));
}

static Value logical_or(void)
{
    const TokenType ops[] = {TOKEN_OR};

    return binary_level(logical_and, ops, sizeof(ops) / sizeof(ops[0]));
}

static Value expression(void)
{
    return logical_or();
}

static void assignment(int require_semicolon)
{
    char name[MAX_TEXT];
    Value index = value_of("");
    int is_array = 0;

    if (token.type != TOKEN_IDENTIFIER) {
        failed = 1;
        return;
    }

    snprintf(name, sizeof(name), "%s", token.lexeme);
    next_token();

    if (match(TOKEN_LEFT_BRACKET)) {
        index = expression();
        expect(TOKEN_RIGHT_BRACKET);
        is_array = 1;
    }

    if (!expect(TOKEN_ASSIGN))
        return;

    Value rhs = expression();

    if (is_array)
        emit("%s[%s] = %s\n", name, index.text, rhs.text);
    else
        emit("%s = %s\n", name, rhs.text);

    if (require_semicolon)
        expect(TOKEN_SEMICOLON);
}

static void statement(void);

static void block(void)
{
    if (!expect(TOKEN_LEFT_BRACE))
        return;

    while (!failed && token.type != TOKEN_RIGHT_BRACE &&
           token.type != TOKEN_EOF) {
        statement();
    }

    expect(TOKEN_RIGHT_BRACE);
}

static void declaration(void)
{
    next_token(); /* Consume int or string. */

    if (token.type != TOKEN_IDENTIFIER) {
        failed = 1;
        return;
    }

    next_token();

    if (match(TOKEN_LEFT_BRACKET)) {
        if (token.type != TOKEN_INT_LITERAL) {
            failed = 1;
            return;
        }

        next_token();
        expect(TOKEN_RIGHT_BRACKET);
    }

    expect(TOKEN_SEMICOLON);
}

static void if_statement(void)
{
    Value condition;
    Value false_label = new_label();
    Value end_label = new_label();

    expect(TOKEN_IF);
    expect(TOKEN_LEFT_PAREN);
    condition = expression();
    expect(TOKEN_RIGHT_PAREN);

    emit("ifFalse %s goto %s\n", condition.text, false_label.text);
    statement();

    if (match(TOKEN_ELSE)) {
        emit("goto %s\n", end_label.text);
        emit("%s:\n", false_label.text);

        statement();

        emit("%s:\n", end_label.text);
    } else {
        emit("%s:\n", false_label.text);
    }
}

static void while_statement(void)
{
    Value start_label = new_label();
    Value end_label = new_label();

    expect(TOKEN_WHILE);

    emit("%s:\n", start_label.text);

    expect(TOKEN_LEFT_PAREN);
    Value condition = expression();
    expect(TOKEN_RIGHT_PAREN);

    emit("ifFalse %s goto %s\n", condition.text, end_label.text);

    statement();

    emit("goto %s\n", start_label.text);
    emit("%s:\n", end_label.text);
}

static void for_statement(void)
{
    Value start_label = new_label();
    Value end_label = new_label();

    expect(TOKEN_FOR);
    expect(TOKEN_LEFT_PAREN);

    if (token.type != TOKEN_SEMICOLON)
        assignment(0);

    expect(TOKEN_SEMICOLON);

    emit("%s:\n", start_label.text);

    if (token.type != TOKEN_SEMICOLON) {
        Value condition = expression();
        emit("ifFalse %s goto %s\n",
             condition.text, end_label.text);
    }

    expect(TOKEN_SEMICOLON);

    /*
       Save update TAC: the update must run after the loop body,
       not before it.
    */
    char update[MAX_OUTPUT];
    size_t saved_length = output_length;

    if (token.type != TOKEN_RIGHT_PAREN)
        assignment(0);

    size_t update_length = output_length - saved_length;

    if (update_length >= sizeof(update)) {
        failed = 1;
        return;
    }

    memcpy(update, output + saved_length, update_length);
    update[update_length] = '\0';

    output_length = saved_length;
    output[output_length] = '\0';

    expect(TOKEN_RIGHT_PAREN);

    statement();

    emit("%s", update);
    emit("goto %s\n", start_label.text);
    emit("%s:\n", end_label.text);
}

static void statement(void)
{
    if (failed)
        return;

    if (token.type == TOKEN_LEFT_BRACE) {
        block();
    } else if (token.type == TOKEN_INT ||
               token.type == TOKEN_STRING) {
        declaration();
    } else if (token.type == TOKEN_IF) {
        if_statement();
    } else if (token.type == TOKEN_WHILE) {
        while_statement();
    } else if (token.type == TOKEN_FOR) {
        for_statement();
    } else if (token.type == TOKEN_IDENTIFIER) {
        assignment(1);
    } else if (match(TOKEN_PRINT)) {
        Value value = expression();
        emit("print %s\n", value.text);
        expect(TOKEN_SEMICOLON);
    } else if (match(TOKEN_READ)) {
        if (token.type != TOKEN_IDENTIFIER) {
            failed = 1;
            return;
        }

        emit("read %s\n", token.lexeme);
        next_token();
        expect(TOKEN_SEMICOLON);
    } else if (match(TOKEN_RETURN)) {
        if (token.type != TOKEN_SEMICOLON) {
            Value value = expression();
            emit("return %s\n", value.text);
        } else {
            emit("return\n");
        }

        expect(TOKEN_SEMICOLON);
    } else {
        failed = 1;
    }
}

void generate_tac(const char *source)
{
    output[0] = '\0';
    output_length = 0;
    temp_count = 0;
    label_count = 0;
    failed = 0;

    lexer_init(source);
    next_token();

    while (!failed && token.type != TOKEN_EOF) {
        if (!match(TOKEN_FUNC)) {
            failed = 1;
            break;
        }

        if (token.type != TOKEN_IDENTIFIER) {
            failed = 1;
            break;
        }

        next_token();

        if (!expect(TOKEN_LEFT_PAREN))
            break;

        /* Skip optional function parameters. */
        while (!failed && token.type != TOKEN_RIGHT_PAREN &&
               token.type != TOKEN_EOF) {
            next_token();
        }

        if (!expect(TOKEN_RIGHT_PAREN))
            break;

        block();
    }

    printf("\n=== Three Address Code ===\n\n");

    if (failed) {
        printf("TAC generation failed: unsupported or invalid construct.\n");
        return;
    }

    printf("%s", output);
}