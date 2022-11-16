#pragma once

#include <vector>
#include <memory>
#include <variant>

namespace ast {

struct LabelDecl {
};

struct Label {
};

struct Unit {
    std::variant<
        LabelDecl
    > inner;

    void visit(auto& v) {
        // v.start(*this);
    }
};

struct SourceFile {
    std::vector<std::unique_ptr<Unit>> units;

    void visit(auto& v) {
        v.start(*this);
        for (const auto& u : units) {
            u->visit(v);
        }
        v.end(*this);
    }
};


}
