#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>

#include "ast.h"
#include "lexer.h"

class Parser {
  public:
    Parser(std::vector<Token> tokens) : cursor(0), tokens(tokens) {}

    void parse();
    void visit(auto& v) {
        assert(this->astRoot != nullptr);
        this->astRoot->visit(v, 0);
    }

  private:
    std::unique_ptr<ast::SourceFile> astRoot;

    // -- production rules
    auto source_file() -> std::unique_ptr<ast::SourceFile>;
    auto unit() -> std::unique_ptr<ast::Unit>;
    auto label_decl() -> std::unique_ptr<ast::LabelDecl>;

    auto instruction() -> std::unique_ptr<ast::Instruction>;
    auto operand() -> std::optional<ast::Operand>;
    auto operand_register() -> std::optional<ast::OperandRegister>;
    auto operand_label() -> std::optional<ast::OperandLabel>;
    auto operand_memory() -> std::optional<ast::OperandMemory>;
    auto operand_memory_postincr() -> std::optional<ast::OperandMemoryPostIncr>;

    auto directive_origin() -> std::unique_ptr<ast::OriginDirective>;
    auto directive_section() -> std::unique_ptr<ast::SectionDirective>;

    // -- error handling
    void error(const std::string& err) {
        // TODO: this is just a temporary hack
        // std::cerr << "parse error near line " << curr().getSrcLoc()->lineno
        //           << ": " << err << '\n';
        fmt::print(fmt::fg(fmt::color::red), "parse error near line {}: {}\n",
                   curr().getSrcLoc()->lineno, err);
        std::exit(1);
    }

    // -- token utilities
    auto curr() -> const Token& { return tokens[cursor]; }
    auto peek(ssize_t offset = 1) -> const Token& {
        return tokens[cursor + offset];
    }
    auto next(size_t incr = 1) -> const Token& {
        cursor += incr;
        if (cursor >= tokens.size())
            cursor = tokens.size() - 1;

        return tokens[cursor];
    }
    auto eof() -> bool { return cursor == tokens.size() - 1; }

    size_t cursor;
    std::vector<Token> tokens;
};
