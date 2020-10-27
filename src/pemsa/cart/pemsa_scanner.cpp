#include "pemsa/cart/pemsa_scanner.hpp"

#include <cstring>
#include <iostream>

PemsaScanner::PemsaScanner(const char *source) {
	this->current = source;
	this->line = 1;
}

static bool isDigit(char c) {
	return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

PemsaToken PemsaScanner::parseIdentifier() {
	while (isAlpha(this->peek()) || isDigit(this->peek())) {
		this->advance();
	}

	return this->makeToken(this->decideIdentifierType());
}

PemsaToken PemsaScanner::parseNumber() {
	while (isDigit(this->peek())) {
		this->advance();
	}

	char c = peek();

	if ((c == '.' || c == 'x' || c == 'b') && isDigit(peekNext())) {
		// Consume the "."
		this->advance();

		while (isDigit(this->peek())) {
			this->advance();
		}
	}

	return makeToken(TOKEN_NUMBER);
}

PemsaToken PemsaScanner::scan() {
	if (this->skipWhitespace()) {
		return this->makeToken(TOKEN_NEW_LINE);
	}

	this->start = this->current;

	if (this->isAtEnd()) {
		return this->makeToken(TOKEN_EOF);
	}

	char c = this->advance();

	if (isAlpha(c)) {
		return this->parseIdentifier();
	}

	if (isDigit(c)) {
		return this->parseNumber();
	}

	switch (c) {
		case '#': return this->makeToken(TOKEN_SHARP);
		case '(': return this->makeToken(TOKEN_LEFT_PAREN);
		case ')': return this->makeToken(TOKEN_RIGHT_PAREN);
		case '{': return this->makeToken(TOKEN_LEFT_BRACE);
		case '}': return this->makeToken(TOKEN_RIGHT_BRACE);
		case ']': return this->makeToken(TOKEN_RIGHT_BRACKET);
		case ';': return this->makeToken(TOKEN_SEMICOLON);
		case '?': return this->makeToken(TOKEN_QUESTION);
		case ',': return this->makeToken(TOKEN_COMMA);
		case '\\': return this->makeToken(TOKEN_BACKWARDS_SLASH);
		case ':': return this->makeToken(this->match(':') ? TOKEN_COLON_COLON : TOKEN_COLON);
		case '+': return this->makeToken(this->match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
		case '-': return this->makeToken(this->match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
		case '/': return this->makeToken(this->match('=') ? TOKEN_SLASH_EQUAL : TOKEN_SLASH);
		case '*': return this->makeToken(this->match('=') ? TOKEN_STAR_EQUAL : TOKEN_STAR);
		case '!': return this->makeToken(this->match('=') ? TOKEN_TILDA_EQUAL : TOKEN_NOT);
		case '=': return this->makeToken(this->match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<': return this->makeToken(this->match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>': return this->makeToken(this->match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

		case '.': {
			if (this->match('.')) {
				return this->makeToken(this->match('.') ? TOKEN_DOT_DOT_DOT : TOKEN_DOT_DOT);
			}

			return this->makeToken(TOKEN_DOT);
		}

		case '\'':
		case '"': {
			char stringStart = c;

			while (this->peek() != stringStart && !this->isAtEnd()) {
				if (this->peek() == '\n') {
					this->line++;
				}

				this->advance();
			}

			if (this->isAtEnd()) {
				return this->makeErrorToken("Unterminated string");
			}

			// The closing quote
			this->advance();
			return this->makeToken(TOKEN_STRING);
		}

		case '[': {
			if (this->peek() == '[') {
				while (!(this->peek() == ']' && this->peekNext() == ']') && !this->isAtEnd()) {
					if (this->peek() == '\n') {
						this->line++;
					}

					this->advance();
				}

				if (this->isAtEnd()) {
					return this->makeErrorToken("Unterminated string");
				}

				this->advance();
				this->advance();

				return this->makeToken(TOKEN_MULTILINE_STRING);
			}

			return this->makeToken(TOKEN_LEFT_BRACKET);
		}

		case -30:
		case -16:
		case -53: {
			int code[3];

			code[0] = c;

			for (int i = 0; i < 2; i++) {
				code[i + 1] = advance();
			}

#define CASE(a, b, c, d, u) if (code[0] == a && code[1] == b && code[2] == c) { return this->makeAsciiToken(d); }
#include "pemsa/cart/pemsa_cases.hpp"
#undef CASE

			break;
		}
	}

	return this->makeErrorToken("Unexpected character");
}

bool PemsaScanner::isAtEnd() {
	return *this->current == '\0';
}

PemsaToken PemsaScanner::makeToken(PemsaTokenType type) {
	PemsaToken token;

	token.type = type;
	token.line = this->line;

	if (type == TOKEN_NEW_LINE) {
		token.start = "\n";
		token.length = 1;
	} else {
		token.start = this->start;
		token.length = (int) (this->current - this->start);
	}

	return token;
}

PemsaToken PemsaScanner::makeErrorToken(const char* message) {
	PemsaToken token;

	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = strlen(message);
	token.line = this->line;

	return token;
}

PemsaToken PemsaScanner::makeAsciiToken(const char *type) {
	PemsaToken token;

	token.type = TOKEN_IDENTIFIER;
	token.start = type;
	token.length = strlen(type);
	token.line = this->line;

	return token;
}

char PemsaScanner::advance() {
	return *this->current++;
}

char PemsaScanner::peek() {
	return *this->current;
}

char PemsaScanner::peekNext() {
	if (isAtEnd()) {
		return '\0';
	}

	return this->current[1];
}

bool PemsaScanner::match(char c) {
	if (this->isAtEnd()) {
		return false;
	}

	if (*this->current != c) {
		return false;
	}

	this->current++;
	return true;
}

bool PemsaScanner::skipWhitespace() {
	while (true) {
		char c = this->peek();

		switch (c) {
			case ' ':
			case '\r':
			case '\t': {
				this->advance();
				break;
			}

			case '\n': {
				this->line++;
				this->advance();

				return true;
			}

			case '-':
			case '/': {
				if (this->peekNext() == c) {
					this->advance();
					this->advance();

					bool multiline = c == '-' && this->peek() == '[' && this->peekNext() == '[';

					if (multiline) {
						this->advance();
						this->advance();

						while (!(this->peek() == ']' && this->peekNext() == ']') && !this->isAtEnd()) {
							this->advance();
						}

						this->advance();
						this->advance();
					} else {
						while (this->peek() != '\n' && !this->isAtEnd()) {
							this->advance();
						}
					}
				} else {
					return false;
				}

				break;
			}

			default: {
				return false;
			}
		}
	}
}

PemsaTokenType PemsaScanner::checkKeyword(int start, int length, const char* rest, PemsaTokenType type) {
	if (this->current - this->start == start + length && memcmp(this->start + start, rest, length) == 0) {
		return type;
	}

	return TOKEN_IDENTIFIER;
}

PemsaTokenType PemsaScanner::decideIdentifierType() {
	switch (this->start[0]) {
		case 'a': return this->checkKeyword(1, 2, "nd", TOKEN_AND);
		case 'b': return this->checkKeyword(1, 4, "reak", TOKEN_BREAK);
		case 'd': return this->checkKeyword(1, 1, "o", TOKEN_DO);
		case 'l': return this->checkKeyword(1, 4, "ocal", TOKEN_LOCAL);
		case 'o': return this->checkKeyword(1, 1, "r", TOKEN_OR);
		case 'u': return this->checkKeyword(1, 4, "ntil", TOKEN_UNTIL);
		case 'w': return this->checkKeyword(1, 4, "hile", TOKEN_WHILE);
		case 'g': return this->checkKeyword(1, 3, "oto", TOKEN_GOTO);

		case 'e': {
			if (this->current - this->start > 4) {
				return this->checkKeyword(1, 5, "lseif", TOKEN_ELSE_IF);
			}

			if (this->current - this->start > 1) {
				switch (this->current[1]) {
					case 'n': return this->checkKeyword(2, 1, "d", TOKEN_END);
					case 'l': return this->checkKeyword(2, 2, "se", TOKEN_ELSE);
				}
			}

			break;
		}

		case 'f': {
			if (this->current - this->start > 1) {
				switch (this->current[1]) {
					case 'a': return this->checkKeyword(2, 3, "lse", TOKEN_FALSE);
					case 'o': return this->checkKeyword(2, 1, "r", TOKEN_FOR);
					case 'u': return this->checkKeyword(2, 6, "nction", TOKEN_FUNCTION);
				}
			}

			break;
		}

		case 'i': {
			if (this->current - this->start > 1) {
				switch (this->current[1]) {
					case 'f': return this->checkKeyword(2, 0, "", TOKEN_IF);
					case 'n': return this->checkKeyword(2, 0, "", TOKEN_IN);
				}
			}

			break;
		}

		case 'n': {
			if (this->current - this->start > 1) {
				switch (this->current[1]) {
					case 'i': return this->checkKeyword(2, 1, "l", TOKEN_NIL);
					case 'o': return this->checkKeyword(2, 1, "t", TOKEN_NOT);
				}
			}

			break;
		}

		case 'r': {
			if (this->current - this->start > 2) {
				switch (this->current[2]) {
					case 'p': return this->checkKeyword(3, 3, "eat", TOKEN_REPEAT);
					case 't': return this->checkKeyword(3, 3, "urn", TOKEN_RETURN);
				}
			}

			break;
		}

		case 't': {
			if (this->current - this->start > 1) {
				switch (this->current[1]) {
					case 'h': return this->checkKeyword(2, 2, "en", TOKEN_THEN);
					case 'r': return this->checkKeyword(2, 2, "ue", TOKEN_TRUE);
				}
			}

			break;
		}
	}

	return TOKEN_IDENTIFIER;
}