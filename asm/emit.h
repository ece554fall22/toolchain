#pragma once

#include <map>

#include <fmt/core.h>

#include "ast.h"
#include <morph/encoder.h>

class EmissionPass {
  public:
    void enter(const ast::Instruction& inst, size_t depth) {
        fmt::print("Instruction\n");
    }

    void enter(const auto& x, size_t depth) {}
    void exit(const auto& x, size_t depth) {}

  private:
    isa::Emitter e;
};
