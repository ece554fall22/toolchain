#pragma once

#include <cassert>
#include <memory>
#include <variant>
#include <vector>

#include "lexer.h"
#include <morph/ty.h>
#include <morph/util.h>
#include <morph/varint.h>

namespace ast {

struct OperandImmediate {
    int64_t val;

    friend std::ostream& operator<<(std::ostream& os,
                                    const ast::OperandImmediate& op) {
        os << "Immediate(" << op.val << ")";
        return os;
    }
};

struct OperandRegister {
    Token tok;
    bool vector;
    uint idx;

    friend std::ostream& operator<<(std::ostream& os,
                                    const ast::OperandRegister& op) {
        os << "Register(" << (op.vector ? "v" : "r") << op.idx << ")";
        return os;
    }
};

struct OperandLabel {
    Token label;

    friend std::ostream& operator<<(std::ostream& os,
                                    const ast::OperandLabel& op) {
        os << "Label(" << op.label.getLexeme() << ")";
        return os;
    }
};

struct OperandMemory {
    OperandRegister base;
    int64_t offset;
    bool increment;

    friend std::ostream& operator<<(std::ostream& os,
                                    const ast::OperandMemory& op) {
        os << "Ptr { base=" << op.base << ", offset=" << op.offset
           << ", incr=" << op.increment << " }";
        return os;
    }
};

struct Operand {
    std::variant<OperandImmediate, OperandLabel, OperandRegister, OperandMemory>
        inner;

    template <typename T> Operand(T&& ld) : inner(std::move(ld)) {}

    template <class T> bool is() const noexcept {
        return std::holds_alternative<T>(inner);
    }

    template <class T> const T& get() const { return std::get<T>(inner); }

    auto asRegIdx() const -> reg_idx {
        return {this->template get<ast::OperandRegister>().idx};
    }

    template <size_t N> auto asSignedImm() const -> s<N> {
        return s<N>(this->template get<ast::OperandImmediate>().val);
    }

    template <size_t N> auto asBitsImm() const -> bits<N> {
        return s<N>(this->template get<ast::OperandImmediate>().val);
    }

    friend std::ostream& operator<<(std::ostream& os, const ast::Operand& op) {
        os << "Arg::";
        std::visit([&](auto&& x) { os << x; }, op.inner);
        return os;
    }
};

struct Instruction {
    Instruction(Token mnemonic, std::vector<Operand>&& operands)
        : mnemonic{mnemonic}, operands(std::move(operands)) {}

    Token mnemonic;
    std::vector<Operand> operands;

    void visit(auto& v, size_t depth) { v.enter(*this, depth); }
};

struct LabelDecl {
    LabelDecl(Token ident) : ident{ident} {}

    Token ident;

    void visit(auto& v, size_t depth) { v.enter(*this, depth); }
};

// struct Label {};

struct OriginDirective {
    OriginDirective(uint64_t origin) : origin{origin} {}

    uint64_t origin;

    void visit(auto& v, size_t depth) { v.enter(*this, depth); }
};

struct SectionDirective {
    SectionDirective(Token name) : name{name} {}

    Token name;

    void visit(auto& v, size_t depth) { v.enter(*this, depth); }
};

struct Unit {
    std::variant<std::unique_ptr<LabelDecl>, std::unique_ptr<Instruction>,
                 std::unique_ptr<OriginDirective>,
                 std::unique_ptr<SectionDirective>>
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

  public:
    ASTPrintVisitor(std::ostream& wtr) : wtr{wtr} {}

    void enter(const ast::LabelDecl& ld, size_t depth) {
        // indent(depth);
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
        wtr << "Unit::";
    }

    void exit(const ast::Unit& unit, size_t depth) {
        indent(depth);
        wtr << "} Unit\n";
    }

    void enter(const ast::Instruction& inst, size_t depth) {
        // indent(depth);
        wtr << "Instruction {\n";

        indent(depth + 1);
        wtr << "mnemonic = " << inst.mnemonic.getLexeme() << "\n";

        indent(depth + 1);
        if (inst.operands.size() == 0) {
            wtr << "operands = {}\n";
        } else {
            wtr << "operands = {\n";
            for (auto arg : inst.operands) {
                indent(depth + 2);
                wtr << arg << "\n";
            }
            indent(depth + 1);
            wtr << "}\n";
        }
    }

    void enter(const ast::OriginDirective& d, size_t depth) {
        wtr << "OriginDirective {\n";
        indent(depth + 1);
        wtr << "origin = " << d.origin << "\n";
    }

    void enter(const ast::SectionDirective& d, size_t depth) {
        wtr << "SectionDirective {\n";
        indent(depth + 1);
        wtr << "." << d.name.getLexeme() << "\n";
    }
};
