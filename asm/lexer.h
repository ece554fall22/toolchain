#pragma once

#include <cstdio>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>

struct SourceLocation {
    SourceLocation(size_t lineno) : lineno{lineno} {}

    size_t lineno;
};

class Token {
  public:
    enum class Kind {
#define TOKEN(ident) ident
#include "tokens.def"
    };

    Token(Kind kind) noexcept : kind{kind} {}
    Token(Kind kind, std::string_view lexeme) noexcept
        : kind{kind}, lexeme{lexeme} {}
    Token(Kind kind, std::string_view lexeme, SourceLocation loc) noexcept
        : kind{kind}, lexeme{lexeme}, loc{loc} {}

    Kind getKind() const noexcept { return kind; }
    bool is(Kind other) const noexcept { return kind == other; }

    std::string_view getLexeme() const noexcept { return lexeme; }
    std::optional<SourceLocation> getSrcLoc() const noexcept { return loc; }

  private:
    Kind kind;
    std::string_view lexeme;
    std::optional<SourceLocation> loc;
};

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind);

class Lexer {
  public:
    Lexer(const char* start) : cursor{start}, lineno{0} {}

    Token next() noexcept;

  private:
    const char* cursor;
    size_t lineno;

    char peek() noexcept { return *cursor; }
    char peek(size_t i) noexcept { return cursor[i]; }
    char eat() noexcept { return *cursor++; }
    void eat(size_t n) noexcept { cursor += n; }

    Token tokAtom(Token::Kind kind) noexcept {
        Token tok(kind, std::string_view{cursor, 1}, SourceLocation(lineno));
        eat();
        return tok;
    }

    Token tokFrom(const char* start, Token::Kind kind) noexcept {
        return Token(kind,
                     std::string_view(start, std::distance(start, cursor)),
                     SourceLocation(lineno));
    }

    Token lexIdentifierOrKeyword(const char* tok_start);
    Token lexNumber(const char* tok_start);
    void eatComment();
};
