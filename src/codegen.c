#include "codegen.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAMES 512
#define MAX_NAME  64
#define MAX_LINE  512

static char names[MAX_NAMES][MAX_NAME];
static int name_count;

static int is_number(const char *text)
{
    if (*text == '-' || *text == '+')
        text++;

    if (*text == '\0')
        return 0;

    while (*text != '\0') {
        if (!isdigit((unsigned char)*text))
            return 0;
        text++;
    }

    return 1;
}

static int is_label_name(const char *text)
{
    return text[0] == 'L' &&
           isdigit((unsigned char)text[1]);
}

static int is_reserved(const char *text)
{
    return strcmp(text, "ifFalse") == 0 ||
           strcmp(text, "goto") == 0 ||
           strcmp(text, "print") == 0 ||
           strcmp(text, "return") == 0 ||
           strcmp(text, "read") == 0 ||
           strcmp(text, "param") == 0 ||
           strcmp(text, "call") == 0;
}

static void add_name(const char *text)
{
    char name[MAX_NAME];
    size_t length = 0;

    while (text[length] != '\0' &&
           (isalnum((unsigned char)text[length]) ||
            text[length] == '_') &&
           length < sizeof(name) - 1) {
        name[length] = text[length];
        length++;
    }

    name[length] = '\0';

    if (length == 0 ||
        is_number(name) ||
        is_reserved(name) ||
        is_label_name(name)) {
        return;
    }

    for (int i = 0; i < name_count; i++) {
        if (strcmp(names[i], name) == 0)
            return;
    }

    if (name_count < MAX_NAMES)
        strcpy(names[name_count++], name);
}

static void collect_names(const char *tac)
{
    name_count = 0;

    const char *cursor = tac;

    while (*cursor != '\0') {
        if (isalpha((unsigned char)*cursor) || *cursor == '_') {
            char word[MAX_NAME];
            size_t length = 0;

            while ((isalnum((unsigned char)*cursor) ||
                    *cursor == '_') &&
                   length < sizeof(word) - 1) {
                word[length++] = *cursor++;
            }

            word[length] = '\0';
            add_name(word);
        } else {
            cursor++;
        }
    }
}

static int load_value(FILE *out, const char *value)
{
    char base[MAX_NAME];
    char index[MAX_NAME];

    if (is_number(value)) {
        fprintf(out, "\tmovl $%s, %%eax\n", value);
        return 1;
    }

    const char *bracket = strchr(value, '[');

    if (bracket != NULL) {
        size_t base_length = (size_t)(bracket - value);

        if (base_length == 0 || base_length >= sizeof(base))
            return 0;

        memcpy(base, value, base_length);
        base[base_length] = '\0';

        const char *close = strchr(bracket + 1, ']');

        if (close == NULL)
            return 0;

        size_t index_length = (size_t)(close - bracket - 1);

        if (index_length == 0 || index_length >= sizeof(index))
            return 0;

        memcpy(index, bracket + 1, index_length);
        index[index_length] = '\0';

        if (is_number(index)) {
            fprintf(out, "\tmovl %s+%d(%%rip), %%eax\n",
                    base, atoi(index) * 4);
        } else {
            fprintf(out, "\tmovl %s(%%rip), %%ecx\n", index);
            fprintf(out, "\tleaq %s(%%rip), %%rdx\n", base);
            fprintf(out, "\tmovl (%%rdx,%%rcx,4), %%eax\n");
        }

        return 1;
    }

    fprintf(out, "\tmovl %s(%%rip), %%eax\n", value);
    return 1;
}

static int store_value(FILE *out, const char *destination)
{
    char base[MAX_NAME];
    char index[MAX_NAME];

    const char *bracket = strchr(destination, '[');

    if (bracket == NULL) {
        fprintf(out, "\tmovl %%eax, %s(%%rip)\n", destination);
        return 1;
    }

    size_t base_length = (size_t)(bracket - destination);

    if (base_length == 0 || base_length >= sizeof(base))
        return 0;

    memcpy(base, destination, base_length);
    base[base_length] = '\0';

    const char *close = strchr(bracket + 1, ']');

    if (close == NULL)
        return 0;

    size_t index_length = (size_t)(close - bracket - 1);

    if (index_length == 0 || index_length >= sizeof(index))
        return 0;

    memcpy(index, bracket + 1, index_length);
    index[index_length] = '\0';

    if (is_number(index)) {
        fprintf(out, "\tmovl %%eax, %s+%d(%%rip)\n",
                base, atoi(index) * 4);
    } else {
        fprintf(out, "\tpushq %%rax\n");
        fprintf(out, "\tmovl %s(%%rip), %%ecx\n", index);
        fprintf(out, "\tleaq %s(%%rip), %%rdx\n", base);
        fprintf(out, "\tpopq %%rax\n");
        fprintf(out, "\tmovl %%eax, (%%rdx,%%rcx,4)\n");
    }

    return 1;
}

static int emit_assignment(FILE *out, char *line)
{
    char lhs[MAX_LINE];
    char rhs[MAX_LINE];

    if (sscanf(line, "%511s = %511[^\n]", lhs, rhs) != 2)
        return 0;

    char *operator_position = NULL;
    const char *selected_operator = NULL;

    const char *operators[] = {
        " == ", " != ", " <= ", " >= ",
        " + ", " - ", " * ", " / ", " % ",
        " < ", " > "
    };

    size_t operator_count =
        sizeof(operators) / sizeof(operators[0]);

    for (size_t i = 0; i < operator_count; i++) {
        operator_position = strstr(rhs, operators[i]);

        if (operator_position != NULL) {
            selected_operator = operators[i];
            break;
        }
    }

    if (selected_operator == NULL) {
        if (rhs[0] == '-') {
            if (!load_value(out, rhs + 1))
                return 0;

            fprintf(out, "\tnegl %%eax\n");
            return store_value(out, lhs);
        }

        if (rhs[0] == '!') {
            if (!load_value(out, rhs + 1))
                return 0;

            fprintf(out, "\ttestl %%eax, %%eax\n");
            fprintf(out, "\tsete %%al\n");
            fprintf(out, "\tmovzbl %%al, %%eax\n");

            return store_value(out, lhs);
        }

        if (!load_value(out, rhs))
            return 0;

        return store_value(out, lhs);
    }

    char left[MAX_LINE];
    char right[MAX_LINE];

    size_t left_length = (size_t)(operator_position - rhs);

    if (left_length == 0 || left_length >= sizeof(left))
        return 0;

    memcpy(left, rhs, left_length);
    left[left_length] = '\0';

    const char *right_start =
        operator_position + strlen(selected_operator);

    if (strlen(right_start) >= sizeof(right))
        return 0;

    strcpy(right, right_start);

    if (!load_value(out, left))
        return 0;

    fprintf(out, "\tpushq %%rax\n");

    if (!load_value(out, right))
        return 0;

    fprintf(out, "\tmovl %%eax, %%ecx\n");
    fprintf(out, "\tpopq %%rax\n");

    if (strcmp(selected_operator, " + ") == 0) {
        fprintf(out, "\taddl %%ecx, %%eax\n");
    } else if (strcmp(selected_operator, " - ") == 0) {
        fprintf(out, "\tsubl %%ecx, %%eax\n");
    } else if (strcmp(selected_operator, " * ") == 0) {
        fprintf(out, "\timull %%ecx, %%eax\n");
    } else if (strcmp(selected_operator, " / ") == 0 ||
               strcmp(selected_operator, " % ") == 0) {
        fprintf(out, "\tcltd\n");
        fprintf(out, "\tidivl %%ecx\n");

        if (strcmp(selected_operator, " % ") == 0)
            fprintf(out, "\tmovl %%edx, %%eax\n");
    } else {
        fprintf(out, "\tcmpl %%ecx, %%eax\n");

        if (strcmp(selected_operator, " == ") == 0)
            fprintf(out, "\tsete %%al\n");
        else if (strcmp(selected_operator, " != ") == 0)
            fprintf(out, "\tsetne %%al\n");
        else if (strcmp(selected_operator, " < ") == 0)
            fprintf(out, "\tsetl %%al\n");
        else if (strcmp(selected_operator, " > ") == 0)
            fprintf(out, "\tsetg %%al\n");
        else if (strcmp(selected_operator, " <= ") == 0)
            fprintf(out, "\tsetle %%al\n");
        else if (strcmp(selected_operator, " >= ") == 0)
            fprintf(out, "\tsetge %%al\n");

        fprintf(out, "\tmovzbl %%al, %%eax\n");
    }

    return store_value(out, lhs);
}

static int emit_instruction(FILE *out, char *line)
{
    size_t length = strlen(line);

    while (length > 0 &&
           isspace((unsigned char)line[length - 1])) {
        line[--length] = '\0';
    }

    while (*line == ' ' || *line == '\t')
        line++;

    if (*line == '\0')
        return 1;

    /* Labels, for example L1: */
    if (line[strlen(line) - 1] == ':') {
        fprintf(out, "%s\n", line);
        return 1;
    }

    /* Unconditional jump */
    if (strncmp(line, "goto ", 5) == 0) {
        fprintf(out, "\tjmp %s\n", line + 5);
        return 1;
    }

    /* Conditional jump */
    if (strncmp(line, "ifFalse ", 8) == 0) {
        char condition[MAX_LINE];
        char label[MAX_LINE];

        if (sscanf(line, "ifFalse %511s goto %511s",
                   condition, label) != 2) {
            return 0;
        }

        if (!load_value(out, condition))
            return 0;

        fprintf(out, "\ttestl %%eax, %%eax\n");
        fprintf(out, "\tje %s\n", label);

        return 1;
    }

    /* Integer output */
    if (strncmp(line, "print ", 6) == 0) {
        if (!load_value(out, line + 6))
            return 0;

        fprintf(out, "\tmovl %%eax, %%esi\n");
        fprintf(out, "\tleaq .LCfmt(%%rip), %%rdi\n");
        fprintf(out, "\txorl %%eax, %%eax\n");
        fprintf(out, "\tcall printf@PLT\n");

        return 1;
    }

    /* Return value */
    if (strncmp(line, "return ", 7) == 0) {
        if (!load_value(out, line + 7))
            return 0;

        fprintf(out, "\tjmp .Lreturn\n");
        return 1;
    }

    if (strcmp(line, "return") == 0) {
        fprintf(out, "\txorl %%eax, %%eax\n");
        fprintf(out, "\tjmp .Lreturn\n");
        return 1;
    }

    if (strncmp(line, "read ", 5) == 0) {
        fprintf(stderr,
                "Unsupported TAC instruction: %s\n", line);
        return 0;
    }

    if (strncmp(line, "param ", 6) == 0 ||
        strstr(line, " = call ") != NULL) {
        fprintf(stderr,
                "Unsupported TAC instruction: %s\n", line);
        return 0;
    }

    return emit_assignment(out, line);
}

int generate_target_code(const char *tac, const char *filename)
{
    if (tac == NULL || filename == NULL || *tac == '\0')
        return 0;

    collect_names(tac);

    FILE *out = fopen(filename, "w");

    if (out == NULL) {
        perror("Could not create assembly file");
        return 0;
    }

    fprintf(out, ".section .data\n");
    fprintf(out, ".LCfmt:\n");
    fprintf(out, "\t.string \"%%d\\n\"\n\n");

    for (int i = 0; i < name_count; i++)
        fprintf(out, ".comm %s,4096,16\n", names[i]);

    fprintf(out, "\n.section .text\n");
    fprintf(out, ".globl main\n");
    fprintf(out, ".extern printf\n");
    fprintf(out, "main:\n");
    fprintf(out, "\tpushq %%rbp\n");
    fprintf(out, "\tmovq %%rsp, %%rbp\n");

    char *copy = malloc(strlen(tac) + 1);

    if (copy == NULL) {
        fclose(out);
        return 0;
    }

    strcpy(copy, tac);

    char *line = strtok(copy, "\n");
    int success = 1;

    while (line != NULL) {
        if (!emit_instruction(out, line)) {
            fprintf(stderr,
                    "Could not translate TAC instruction: %s\n",
                    line);
            success = 0;
            break;
        }

        line = strtok(NULL, "\n");
    }

    if (success) {
        fprintf(out, "\n.Lreturn:\n");
        fprintf(out, "\tleave\n");
        fprintf(out, "\tret\n");
        fprintf(out, ".section .note.GNU-stack,\"\",@progbits\n");
    }

    free(copy);

    if (fclose(out) != 0)
        success = 0;

    if (!success)
        remove(filename);

    return success;
}