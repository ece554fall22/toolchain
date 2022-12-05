#pragma once

#include <cstdio>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/color.h>
#include <fmt/ostream.h>

#include <morph/util.h>

struct SourceLocation {
    SourceLocation(size_t lineno) : lineno{lineno} {}

    size_t lineno;
};

struct Token {
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
    bool isNot(Kind other) const noexcept { return !is(other); }
    bool isEoF() const noexcept { return is(Kind::ENDOFFILE); }
    bool isEndOfLine() const noexcept { return isEoF() || is(Kind::LINEBREAK); }
    bool isIntegerLiteral() const noexcept {
        return is(Kind::INTEGER_DEC) || is(Kind::INTEGER_HEX) ||
               is(Kind::INTEGER_BIN);
    }

    std::string_view getLexeme() const noexcept { return lexeme; }
    std::optional<SourceLocation> getSrcLoc() const noexcept { return loc; }

  private:
    Kind kind;
    std::string_view lexeme;
    std::optional<SourceLocation> loc;
};

std::optional<int64_t> parseIntegerToken(const Token& tok);

std::ostream& operator<<(std::ostream& os, const Token::Kind& kind);
template <> struct fmt::formatter<Token::Kind> : ostream_formatter {};

class Lexer {
  public:
    Lexer(const char* start) : cursor{start}, lineno{1} {}

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

    void error(const std::string& err) {
        fmt::print(fmt::fg(fmt::color::red), "lex error near line {}: {}\n",
                   lineno, err);
        std::exit(1);
    }
};

inline void dumpTokens(std::vector<Token>& tokens) {
    for (auto token : tokens) {
        auto srcLoc = token.getSrcLoc();
        if (!srcLoc)
            panic();

        if (token.is(Token::Kind::LINEBREAK) ||
            token.is(Token::Kind::ENDOFFILE)) {
            fmt::print("{:>5} {:>12}\n", srcLoc->lineno, token.getKind());
        } else if (token.isIntegerLiteral()) {
            fmt::print("{:>5} {:>12}: `{}` => {:#x}\n", srcLoc->lineno,
                       token.getKind(), token.getLexeme(),
                       *parseIntegerToken(token));
        } else {
            fmt::print("{:>5} {:>12}: `{}`\n", srcLoc->lineno, token.getKind(),
                       token.getLexeme());
        }
    }
}
