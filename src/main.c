#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "tac.h"
#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <io.h>
    #define DUP _dup
    #define DUP2 _dup2
    #define FILENO _fileno
    #define CLOSE_FD _close
#else
    #include <unistd.h>
    #define DUP dup
    #define DUP2 dup2
    #define FILENO fileno
    #define CLOSE_FD close
#endif

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        printf("Could not open source file: %s\n", filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long size = ftell(file);

    if (size < 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = malloc((size_t)size + 1);

    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t count = fread(buffer, 1, (size_t)size, file);

    if (ferror(file)) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[count] = '\0';
    fclose(file);

    return buffer;
}

static int print_tokens(const char *source)
{
    Token token;
    int valid = 1;

    lexer_init(source);

    printf("=== Lexical Analysis ===\n\n");
    printf("%-6s %-22s %s\n", "Line", "Token Type", "Lexeme");
    printf("------------------------------------------------------------\n");

    while (1) {
        token = lexer_next_token();

        printf("%-6d %-22s %s\n",
               token.line,
               token_type_name(token.type),
               token.lexeme);

        if (token.type == TOKEN_ERROR) {
            valid = 0;
            break;
        }

        if (token.type == TOKEN_EOF)
            break;
    }

    printf("\n");
    return valid;
}

/*
 * Capture the output printed by the existing TAC generator.
 * The TAC implementation itself does not need to be rewritten.
 */
static char *capture_tac(const char *source)
{
    FILE *temporary = tmpfile();

    if (temporary == NULL) {
        perror("Could not create temporary TAC file");
        return NULL;
    }

    fflush(stdout);

    int saved_stdout = DUP(FILENO(stdout));

    if (saved_stdout == -1) {
        fclose(temporary);
        perror("Could not save stdout");
        return NULL;
    }

    if (DUP2(FILENO(temporary), FILENO(stdout)) == -1) {
        CLOSE_FD(saved_stdout);
        fclose(temporary);
        perror("Could not redirect stdout");
        return NULL;
    }

    generate_tac(source);

    fflush(stdout);

    int restore_ok = DUP2(saved_stdout, FILENO(stdout));
    CLOSE_FD(saved_stdout);

    if (restore_ok == -1) {
        fclose(temporary);
        perror("Could not restore stdout");
        return NULL;
    }

    if (fseek(temporary, 0, SEEK_END) != 0) {
        fclose(temporary);
        return NULL;
    }

    long length = ftell(temporary);

    if (length < 0 || fseek(temporary, 0, SEEK_SET) != 0) {
        fclose(temporary);
        return NULL;
    }

    char *result = malloc((size_t)length + 1);

    if (result == NULL) {
        fclose(temporary);
        return NULL;
    }

    size_t count = fread(result, 1, (size_t)length, temporary);

    result[count] = '\0';

    fclose(temporary);

    return result;
}

/*
 * Remove the display heading so that the backend receives
 * TAC instructions only.
 */
static const char *get_tac_instructions(const char *captured)
{
    const char *heading = strstr(captured, "=== Three Address Code ===");

    if (heading != NULL) {
        heading += strlen("=== Three Address Code ===");

        while (*heading == '\r' || *heading == '\n' ||
               *heading == ' ' || *heading == '\t') {
            heading++;
        }

        return heading;
    }

    return captured;
}

static int contains_tac_failure(const char *text)
{
    return strstr(text, "TAC generation failed") != NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    char *source = read_file(argv[1]);

    if (source == NULL)
        return 1;

    printf("========== MICRO C COMPILER ==========\n\n");

    /* Stage 1: Lexical Analysis */
    if (!print_tokens(source)) {
        printf("Lexical Analysis: FAILED\n");
        free(source);
        return 1;
    }

    /* Stage 2: Syntax Analysis */
    lexer_init(source);

    if (parse_program()) {
        printf("=== Syntax Analysis ===\n");
        printf("Syntax Analysis: SUCCESS\n");
        printf("Program syntax is valid.\n\n");
    } else {
        printf("=== Syntax Analysis ===\n");
        printf("Syntax Analysis: FAILED\n");
        printf("%s\n", parser_error());

        free(source);
        return 1;
    }

    /* Stage 3: Semantic Analysis */
    if (semantic_analysis(source)) {
        printf("=== Semantic Analysis ===\n");
        printf("Semantic Analysis: SUCCESS\n");
        printf("No semantic errors found.\n\n");
    } else {
        printf("=== Semantic Analysis ===\n");
        printf("Semantic Analysis: FAILED\n");
        printf("%s\n", semantic_error());

        free(source);
        return 1;
    }

    /* Stage 4: Three Address Code */
    char *captured = capture_tac(source);

    if (captured == NULL) {
        printf("Could not capture TAC output.\n");
        free(source);
        return 1;
    }

    printf("%s", captured);

    if (contains_tac_failure(captured)) {
        printf("\nTarget code generation skipped because TAC generation failed.\n");
        free(captured);
        free(source);
        return 1;
    }

    const char *instructions = get_tac_instructions(captured);

    if (*instructions == '\0') {
        printf("No TAC instructions were generated.\n");
        free(captured);
        free(source);
        return 1;
    }

    /* Stage 5: Target Code Generation */
    printf("\n=== Target Code Generation ===\n");

    if (!generate_target_code(instructions, "output.s")) {
        printf("Target code generation: FAILED\n");
        printf("See the error message above for details.\n");

        free(captured);
        free(source);
        return 1;
    }

    printf("Target code generation: SUCCESS\n");
    printf("Assembly saved to: output.s\n\n");

    FILE *assembly = fopen("output.s", "r");

    if (assembly == NULL) {
        printf("Assembly was generated but could not be reopened for display.\n");
        free(captured);
        free(source);
        return 1;
    }

    printf("=== Generated x86-64 Assembly ===\n\n");

    char line[512];

    while (fgets(line, sizeof(line), assembly) != NULL)
        fputs(line, stdout);

    fclose(assembly);

    free(captured);
    free(source);

    return 0;
}