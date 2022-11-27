#pragma once

#include "ast.h"

struct SemanticsError {
    SemanticsError(size_t lineno, std::string err) : lineno(lineno), err(err) {}

    size_t lineno;
    std::string err;
};

enum class OperandType {
    Register,
    Immediate,
    Label,
    Memory,
};

class SemanticsPass {
  public:
    SemanticsPass() : errors{} {}

    void enter(const ast::Instruction& inst, size_t depth);

    void enter(const auto& x, size_t depth) {}
    void exit(const auto& x, size_t depth) {}

    const std::vector<SemanticsError>& getErrors() const { return errors; }

  private:
    std::vector<SemanticsError> errors;

    void error(size_t lineno, const std::string& err) {
        // fmt::print(fmt::fg(fmt::color::red), "semantics error: {}\n", err);
        // std::exit(1);
        errors.emplace_back(lineno, err);
    }
};
