#pragma once

#include <map>

#include <fmt/core.h>

#include <morph/encoder.h>
#include "ast.h"
#include "symtab.h"

class EmissionPass {
  public:
    explicit EmissionPass(const SymbolTable& symtab) : emitter{}, symtab{symtab} {}

    void enter(const ast::Instruction& inst, size_t depth);

    void enter(const auto& x, size_t depth) {}
    void exit(const auto& x, size_t depth) {}

    auto getData() -> const auto& { return emitter.getData(); }

  private:
    void error(const std::string& err) {
        fmt::print(fmt::fg(fmt::color::red), "emission error: {}\n", err);
        std::exit(1);
    }

    isa::Emitter emitter;
    const SymbolTable& symtab;
};
