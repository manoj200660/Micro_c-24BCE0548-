#ifndef LEXER_H
#define LEXER_H

#include "token.h"

void lexer_init(const char *input);
Token lexer_next_token(void);

#endif