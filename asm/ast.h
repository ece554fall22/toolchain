#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "lexer.h"
#include <morph/util.h>

namespace ast {

struct InstructionArg {
    std::variant<int64_t, Token> inner;

    friend std::ostream& operator<<(std::ostream& os,
                                    const ast::InstructionArg& arg) {
        os << "Arg::";
        std::visit(overloaded{
                       [&](int64_t v) { os << "Int(" << v << ")"; },
                       [&](const Token& v) {
                           os << "Label(" << v.getLexeme() << ")";
                       },
                   },
                   arg.inner);
        return os;
    }
};

struct Instruction {
    Instruction(Token mnemonic, std::vector<InstructionArg>&& args)
        : mnemonic{mnemonic}, args(std::move(args)) {}

    Token mnemonic;
    std::vector<InstructionArg> args;

    void visit(auto& v, size_t depth) { v.enter(*this, depth); }
};

struct LabelDecl {
    LabelDecl(Token ident) : ident{ident} {}

    Token ident;

    void visit(auto& v, size_t depth) { v.enter(*this, depth); }
};

struct Label {};

struct Unit {
    std::variant<std::unique_ptr<LabelDecl>, std::unique_ptr<Instruction>>
        inner;

    template <typename T>
    Unit(std::unique_ptr<T>&& ld) : inner(std::move(ld)) {}

    void visit(auto& v, size_t depth) {
        v.enter(*this, depth);
        std::visit(
            [&](auto&& x) {
                assert(x != nullptr);
                x->visit(v, depth);
            },
            inner);
        v.exit(*this, depth);
    }
};

struct SourceFile {
    std::vector<std::unique_ptr<Unit>> units;

    void visit(auto& v, size_t depth) {
        v.enter(*this, depth);
        for (const auto& u : units) {
            u->visit(v, depth + 1);
        }
        v.exit(*this, depth);
    }
};

} // namespace ast

class ASTPrintVisitor {
  private:
    std::ostream& wtr;
    auto indent(size_t depth) {
        for (auto i = 0; i < depth; i++) {
            wtr << "  ";
        }
    }
    // auto visit()

  public:
    ASTPrintVisitor(std::ostream& wtr) : wtr{wtr} {}

    void enter(const ast::LabelDecl& ld, size_t depth) {
        indent(depth);
        wtr << "LabelDecl { ident = `" << ld.ident.getLexeme() << "` }\n";
    }

    void enter(const ast::SourceFile& sf, size_t depth) {
        indent(depth);
        wtr << "SourceFile {\n";
    }

    void exit(const ast::SourceFile& sf, size_t depth) {
        indent(depth);
        wtr << "} SourceFile\n";
    }

    void enter(const ast::Unit& unit, size_t depth) {
        indent(depth);
        wtr << "Unit {\n";
    }

    void exit(const ast::Unit& unit, size_t depth) {
        indent(depth);
        wtr << "} Unit\n";
    }

    void enter(const ast::Instruction& inst, size_t depth) {
        indent(depth);
        wtr << "Instruction {\n";

        indent(depth + 1);
        wtr << "mnemonic = " << inst.mnemonic.getLexeme() << "\n";

        indent(depth + 1);
        if (inst.args.size() == 0) {
            wtr << "args = {}\n";
        } else {
            wtr << "args = {\n";
            for (auto arg : inst.args) {
                indent(depth + 2);
                wtr << arg << "\n";
            }
            indent(depth + 1);
            wtr << "}\n";
        }

        indent(depth);
        wtr << "} Instruction\n";
    }
};
