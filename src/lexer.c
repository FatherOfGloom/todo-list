#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#include "./lexer.h"

const char *get_token_type_name(Token_Type type) {
    switch (type) {
        case TOKEN_END:
            return "end of content";
        case TOKEN_INVALID:
            return "invalid token";
        case TOKEN_NULL:
            return "null token";
        case TOKEN_SYMBOL:
            return "symbol";
        case TOKEN_COMMA:
            return "comma token"; 
        case TOKEN_BRACKET_OPEN: 
            return "open bracket";
        case TOKEN_BRACKET_CLOSE:
			return "close bracket";
		case TOKEN_BRACE_OPEN: 
			return "open brace";
		case TOKEN_BRACE_CLOSE:
			return "close brace";
		case TOKEN_INTEGER:
			return "integer token";
		case TOKEN_STRING: 
			return "string literal";
		case TOKEN_BOOL:
			return "bool token";
		case TOKEN_COLON:
			return "colon token";
		default:
			puts("get_token_type_name: unexpected Token_Type");
			break;
	}
    return NULL;
}

Lexer lexer_new(const char *content, size_t content_len) {
    Lexer l = {0};
    l.content_len = content_len;
    l.content = content;
    return l;
}

bool is_integer_start(char c) { 
    return isdigit(c) && c != '0';
}

bool is_symbol_start(char c) {
    return isalpha(c) || c == '_';
}

bool is_symbol(char c) {
    return isalnum(c) || c == '_';
}

void lexer_eat_chars(Lexer *l, size_t count) {
	for (size_t i = 0; i < count; ++i) { 
		assert(l->cursor < l->content_len);
		char c = l->content[l->cursor];
		l->cursor += 1;
		if (c == '\n') { 
			l->line += 1;
			l->bol = l->cursor;
		}
	}
}

void lexer_trim_left(Lexer *l) {
    while(l->cursor < l->content_len && isspace(l->content[l->cursor])) {
        lexer_eat_chars(l, 1);
    }
}

bool is_keyword(Token *t, const char *keyword) {
	size_t keyword_len = strlen(keyword);
	return (keyword_len == t->string_len && memcmp(keyword, t->string, keyword_len) == 0);
}

Token lexer_next(Lexer *l) { 
    lexer_trim_left(l);
    Token token = {
        .string = &l->content[l->cursor]
    };
    if (l->cursor >= l->content_len) {
        return token;
    }
    switch (l->content[l->cursor]) {
        case '#': 
            token.type = TOKEN_NULL;
            break;
		case '=':
			token.type = TOKEN_EQUALS;
			break;
        case ',':
            token.type = TOKEN_COMMA;
            break;
		case '[':
			token.type = TOKEN_BRACKET_OPEN;
			break;
		case ']':
			token.type = TOKEN_BRACKET_CLOSE;
			break;
		case '{':
			token.type = TOKEN_BRACE_OPEN;
			break;
		case '}':
			token.type = TOKEN_BRACE_CLOSE;
			break;
		case ':':
			token.type = TOKEN_COLON;
			break;
		case '"':
			token.type = TOKEN_STRING;
			lexer_eat_chars(l, 1);
			while(l->cursor < l->content_len && l->content[l->cursor] != '"' && l->content[l->cursor] != '\n') {
				lexer_eat_chars(l, 1);
			}
			if (l->cursor < l->content_len && l->content[l->cursor] == '"') {
				lexer_eat_chars(l, 1);
			} else {
				token.type = TOKEN_INVALID;
			}
			token.string_len = &l->content[l->cursor] - token.string;
			return token;
		default:
			if (is_symbol_start(l->content[l->cursor])) {
				token.type = TOKEN_SYMBOL;
				while(l->cursor < l->content_len && is_symbol(l->content[l->cursor])) {
					lexer_eat_chars(l, 1);
				}
				token.string_len = &l->content[l->cursor] - token.string;
				if (is_keyword(&token, "true") || is_keyword(&token, "false")) {
					token.type = TOKEN_BOOL;
				}
				return token;
			}
			if (is_integer_start(l->content[l->cursor])) {
				token.type = TOKEN_INTEGER;
				while(l->cursor < l->content_len && isdigit(l->content[l->cursor])) {
					lexer_eat_chars(l, 1);
				}
				token.string_len = &l->content[l->cursor] - token.string;
				return token;
			}
			token.type = TOKEN_INVALID; 
			break;
    }
	lexer_eat_chars(l, 1);
    token.string_len = 1;
    return token;
}

