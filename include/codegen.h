#ifndef CODEGEN_H
#define CODEGEN_H

/*
 * Convert supported TAC instructions into x86-64 AT&T assembly.
 *
 * tac      : TAC instruction text
 * filename : destination assembly file
 *
 * Returns 1 on success and 0 on failure.
 */
int generate_target_code(const char *tac, const char *filename);

#endif