#pragma once

#include <iterator>
#include <string>
#include <string_view>
#include <cstdio>

class Token {
public:
    enum class Kind {
        #define TOKEN(ident) ident
        #include "tokens.def"
    };

    Token(Kind kind) noexcept : kind{kind} {}
    Token(Kind kind, std::string_view lexeme) noexcept : kind{kind}, lexeme{lexeme} {}

    Kind getKind() const noexcept { return kind; }
    bool is(Kind other) const noexcept { return kind == other; }

    std::string_view getLexeme() const noexcept { return lexeme; }

private:
    Kind kind;
    std::string_view lexeme;
};

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind);

class Lexer {
public:
    Lexer(const char* start) : cursor{start} {}

    Token next() noexcept;

private:
    const char* cursor;

    char peek() noexcept { return *cursor; }
    char peek(size_t i) noexcept { return cursor[i]; }
    char eat() noexcept { return *cursor++; }
    void eat(size_t n) noexcept { cursor += n; }

    Token tokAtom(Token::Kind kind) noexcept {
        Token tok(kind, std::string_view{cursor, 1});
        eat();
        return tok;
    }

    Token tokFrom(const char* start, Token::Kind kind) noexcept {
        return Token(kind, std::string_view(start, std::distance(start, cursor)));
    }

    Token lexIdentifierOrKeyword(const char* tok_start);
    Token lexNumber(const char* tok_start);
    void eatComment();
};
