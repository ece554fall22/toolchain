#include "lexer.h"

#include <cassert>
#include <cctype>
#include <iostream>

Token Lexer::next() noexcept {
    while (true) {
        switch (peek()) {
        default:
            if (isalpha(peek()) || peek() == '.')
                return lexIdentifierOrKeyword(cursor);

            return tokAtom(Token::Kind::UNEXPECTED);

        case '\0':
            return tokAtom(Token::Kind::ENDOFFILE);

        case ' ':
        case '\t':
        case '\n':
        case '\r':
            // skip whitespace
            eat();
            continue;

        case ';':
            eatComment();
            continue;

        case ':':
            return tokAtom(Token::Kind::COLON);
        case ',':
            return tokAtom(Token::Kind::COMMA);
        case '[':
            return tokAtom(Token::Kind::L_SQUARE);
        case '{':
            return tokAtom(Token::Kind::L_BRACE);
        case ']':
            return tokAtom(Token::Kind::R_SQUARE);
        case '}':
            return tokAtom(Token::Kind::R_BRACE);
        case '#':
            return tokAtom(Token::Kind::HASH);
        case '+':
            if (peek(+1) == '=') {
                // `+=`
                const char* start = cursor;
                eat(2);
                return tokFrom(start, Token::Kind::PLUSEQUAL);
            }
            return tokAtom(Token::Kind::PLUS);

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return lexNumber(cursor);
        }
    }
}

bool isidchar(char x) {
    return isalpha(x) || isdigit(x) || x == '_' || x == '$' || x == '.';
}

Token Lexer::lexIdentifierOrKeyword(const char* tok_start) {
    while (isidchar(peek())) {
        eat();
    }

    return tokFrom(tok_start, Token::Kind::IDENTIFIER);
}

Token Lexer::lexNumber(const char* tok_start) {
    assert(isdigit(peek()));

    // hex!
    if (peek() == '0' && peek(+1) == 'x') {
        cursor += 2;

        while (isxdigit(peek()))
            eat();

        return tokFrom(tok_start, Token::Kind::INTEGER);
    }

    // normal case: decimals
    while (isdigit(peek()))
        eat();

    // TODO: float literals! exponentials!

    return tokFrom(tok_start, Token::Kind::INTEGER);
}

void Lexer::eatComment() {
    while (true) {
        switch (eat()) {
        case '\n':
        case '\r':
            // end of comment!
            return;

        case '\0':
            // rewind so we can emit the EOF token on \0
            cursor--;
            return;

        default:
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind) {
    static const std::string names[] = {
#define TOKEN(ident) #ident
#include "tokens.def"
    };

    return os << names[static_cast<int>(kind)];
}
