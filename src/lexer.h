#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_END = 0,
    TOKEN_INVALID,
    TOKEN_NULL,
    TOKEN_SYMBOL,
    TOKEN_COMMA,
    TOKEN_BRACKET_OPEN, 
    TOKEN_BRACKET_CLOSE,
	TOKEN_BRACE_OPEN,
	TOKEN_BRACE_CLOSE,
    TOKEN_INTEGER,
	TOKEN_STRING,
	TOKEN_BOOL,
	TOKEN_COLON,
	TOKEN_EQUALS
} Token_Type;

typedef struct Token {
    const char *string;
    size_t string_len;
    Token_Type type;
} Token;

typedef struct Tokens {
	Token *items;
	size_t count;
	size_t capacity;
} Tokens;

typedef struct Lexer {
    const char *content;
    size_t content_len; 
    size_t cursor;
    size_t line;
    size_t bol;
} Lexer;

Lexer lexer_new(const char *content, size_t content_len);
Token lexer_next(Lexer *l);
const char *get_token_type_name(Token_Type type);
bool is_keyword(Token *t, const char *keyword);

#endif
