#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstdlib>
#include <iostream>

#include "lexer.h"
#include "ast.h"

class Parser {
  public:
    Parser(std::vector<Token> tokens) : cursor(0), tokens(tokens) {}

    void parse();

  private:
    // -- production rules
    auto source_file() -> std::unique_ptr<ast::SourceFile>;
    auto unit() -> std::unique_ptr<ast::Unit>;
    auto label_decl() -> std::unique_ptr<ast::LabelDecl>;

    // -- error handling
    auto error(const std::string& err) {
        // TODO: this is just a temporary hack
        std::cerr << "parse error: " << err << '\n';
        std::exit(1);
    }

    // -- token utilities
    auto curr() -> const Token& { return tokens[cursor]; }
    auto peek(ssize_t offset = 1) -> const Token& { return tokens[cursor+offset]; }
    auto next(size_t incr = 1) -> const Token& {
        cursor += incr;
        if (cursor > tokens.size()) throw std::runtime_error("oops");

        return tokens[cursor];
    }
    auto eof() -> bool { return cursor == tokens.size(); }

    size_t cursor;
    std::vector<Token> tokens;
};
