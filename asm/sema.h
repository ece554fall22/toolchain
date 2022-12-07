#pragma once

#include <utility>

#include "ast.h"

struct SemanticsError {
    SemanticsError(size_t lineno, std::string err)
        : lineno(lineno), err(std::move(err)) {}

    size_t lineno;
    std::string err;
};

enum class OperandType {
    ScalarRegister,
    VectorRegister,
    VectorMask,
    Immediate,
    Label,
    Memory,
    VectorMemoryImmIncr,
    VectorMemoryRegIncr,
};

class SemanticsPass {
  public:
    SemanticsPass() : errors{} {}

    void enter(const ast::Instruction& inst, size_t depth);

    void enter(const auto& x, size_t depth) {}
    void exit(const auto& x, size_t depth) {}

    [[nodiscard]] const std::vector<SemanticsError>& getErrors() const {
        return errors;
    }

    static void dumpSemanticsTable(std::ostream& os);

  private:
    std::vector<SemanticsError> errors;

    void error(size_t lineno, const std::string& err) {
        // fmt::print(fmt::fg(fmt::color::red), "semantics error: {}\n", err);
        // std::exit(1);
        errors.emplace_back(lineno, err);
    }
};
