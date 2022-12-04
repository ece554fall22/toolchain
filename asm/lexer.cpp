#include "lexer.h"

#include <cassert>
#include <cctype>
#include <charconv>
#include <iostream>

#include <fmt/core.h>

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
            // skip whitespace
            eat();
            continue;
        case '\n':
        case '\r': {
            // omg a newline? 😳
            auto t = tokAtom(Token::Kind::LINEBREAK);
            lineno++;
            return t;
        }

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
        case '%':
            return tokAtom(Token::Kind::PERCENT);
        case '#':
            return tokAtom(Token::Kind::HASH);
        case '+':
            if (peek(+1) == '=') {
                // `+=`
                const char* start = cursor;
                eat(2);
                return tokFrom(start, Token::Kind::PLUSEQUAL);
            } else {
                return tokAtom(Token::Kind::PLUS);
            }
        case '-':
            if (isdigit(peek(+1))) {
                // fall through. is sign-pfxed literal
            } else {
                return tokAtom(Token::Kind::MINUS);
            }

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

bool isbdigit(char x) { return x == '0' || x == '1'; }

Token Lexer::lexIdentifierOrKeyword(const char* tok_start) {
    while (isidchar(peek())) {
        eat();
    }

    return tokFrom(tok_start, Token::Kind::IDENTIFIER);
}

Token Lexer::lexNumber(const char* tok_start) {
    if (peek() == '-' || peek() == '+')
        eat(); // can be prefixed with sign

    assert(isdigit(peek()));

    // hex!
    if (peek() == '0' && peek(+1) == 'x') {
        cursor += 2;

        if (!isxdigit(peek())) {
            error(fmt::format("hex literal prefix `0x` on line {} must be "
                              "followed by hex characters",
                              lineno));
        }

        while (isbdigit(peek()))
            eat();

        return tokFrom(tok_start, Token::Kind::INTEGER_HEX);
    } else if (peek() == '0' && peek(+1) == 'b') {
        cursor += 2;

        if (!isbdigit(peek())) {
            error(fmt::format("bin literal prefix `0b` on line {} must be "
                              "followed by binary characters",
                              lineno));
        }

        while (isbdigit(peek()))
            eat();

        return tokFrom(tok_start, Token::Kind::INTEGER_BIN);
    }

    // normal case: decimals
    while (isdigit(peek()))
        eat();

    // TODO: float literals! exponentials!

    return tokFrom(tok_start, Token::Kind::INTEGER_DEC);
}

void Lexer::eatComment() {
    while (true) {
        switch (peek()) {
        case '\n':
        case '\r':
            // end of comment!
            lineno++;
            return;

        case '\0':
            // rewind so we can emit the EOF token on \0
            // cursor--;
            return;

        default:
            eat();
            break;
        }
    }
}

std::optional<int64_t> parseIntegerToken(const Token& tok) {
    int64_t val;
    auto span = tok.getLexeme();
    std::from_chars_result res{};
    if (tok.is(Token::Kind::INTEGER_DEC)) {
        bool sign = false; // positive
        if (span[0] == '+' || span[0] == '-') {
            if (span[0] == '-')
                sign = true;
            span.remove_prefix(1);
        }

        res = std::from_chars(span.data(), span.data() + span.size(), val, 10);
        if (sign)
            val = -val;
    } else if (tok.is(Token::Kind::INTEGER_HEX)) {
        bool sign = false; // positive
        if (span[0] == '+' || span[0] == '-') {
            if (span[0] == '-')
                sign = true;
            span.remove_prefix(1);
        }

        span.remove_prefix(2);
        res = std::from_chars(span.data(), span.data() + span.size(), val, 16);
        if (sign)
            val = -val;
    } else if (tok.is(Token::Kind::INTEGER_BIN)) {
        span.remove_prefix(2);
        res = std::from_chars(span.data(), span.data() + span.size(), val, 2);
    } else {
        return std::nullopt;
    }

    if (res.ec == std::errc{} && res.ptr == (span.data() + span.size())) {
        return val;
    } else {
        return std::nullopt;
    }
}

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind) {
    static const std::string names[] = {
#define TOKEN(ident) #ident
#include "tokens.def"
    };

    return os << names[static_cast<int>(kind)];
}
