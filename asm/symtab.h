#pragma once

#include <cstdint>
#include <map>
#include <ostream>
#include <compare>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "ast.h"
#include <morph/ty.h>

// struct Symbol {
//     std::string ident;
//     // why not smart? append-only symbol table; shared_ptr is not right
//     model. Symbol* parent;
//
//     friend std::ostream& operator<<(std::ostream& os, const Symbol& l) {
//         if (l.parent) {
//             os << *l.parent << ".";
//         }
//         os << l.ident;
//         return os;
//     }
// };

struct Symbol {
    Symbol(std::string ident, uint64_t addr) : ident{ident}, addr{addr} {}

    std::string ident;
    uint64_t addr;
    friend std::ostream& operator<<(std::ostream& os, const Symbol& l) {
        os << l.ident;
        return os;
    }
};

class SymbolTable {
  public:
    SymbolTable() : table{} {}

    auto get(const std::string_view& ident) const -> std::optional<Symbol> {
        auto it = table.find(ident);
        if (it != table.end()) {
            return it->second;
        } else {
            return std::nullopt;
        }
    }
    void insert(Symbol&& sym) { table.insert({sym.ident, sym}); }

    friend std::ostream& operator<<(std::ostream& os,
                                    const SymbolTable& table) {
        os << "symbol table\n"
              "------------------------------------\n";

        for (const auto& p : table.table) {
            fmt::print(os, "{:>20} : {:#x}\n", p.second.ident, p.second.addr);
        }

        return os;
    }

  private:
    std::map<std::string, Symbol, std::less<>> table;
};

enum class Section {
    text,
    data,
};
inline std::ostream& operator<<(std::ostream& os, const Section s) {
    switch (s) {
    case Section::text:
        os << "text";
        break;
    case Section::data:
        os << "data";
        break;
    }
    return os;
}

class LabelVisitor {
  public:
    LabelVisitor()
        : currAddress{0}, currentParentLabel{nullptr},
          currentSection{}, symtab{} {}

    void enter(const ast::SectionDirective& sd, size_t depth);
    void enter(const ast::LabelDecl& lbl, size_t depth);
    void enter(const ast::Instruction& lbl, size_t depth);

    void enter(const auto& x, size_t depth) {}
    void exit(const auto& x, size_t depth) {}

    auto getSymtab() -> const SymbolTable& { return symtab; }

  private:
    //    std::unique_ptr<ast::LabelDecl> currentLabel;
    uint64_t currAddress;

    std::unique_ptr<ast::LabelDecl> currentParentLabel;
    //    std::optional<uint64_t> currentLabelAddress;

    std::optional<Section> currentSection;

    SymbolTable symtab;

    void error(const std::string& err) {
        fmt::print(fmt::fg(fmt::color::red), "emission error: {}\n", err);
        std::exit(1);
    }
};