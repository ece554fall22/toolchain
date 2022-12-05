#include "symtab.h"

void LabelVisitor::enter(const ast::LabelDecl& ld, size_t depth) {
    auto ident = ld.ident.getLexeme();
    std::string canonicalIdent = std::string(ident);
    // is this a local label?
    if (ident.starts_with('.')) {
        if (!currentParentLabel) {
            error(fmt::format(
                "local label {} must have a parent top-level label", ident));
        }

        canonicalIdent =
            std::string(currentParentLabel->ident.getLexeme()) + canonicalIdent;
    } else {
        // parent label
        currentParentLabel = std::make_unique<ast::LabelDecl>(ld.ident);
    }

    symtab.insert({canonicalIdent, currAddress});
}

void LabelVisitor::enter(const ast::SectionDirective& sd, size_t depth) {
    if (sd.name.getLexeme() == "text") {
        currentSection = Section::text;
    } else if (sd.name.getLexeme() == "data") {
        currentSection = Section::data;
    } else {
        error(fmt::format(
            "section directive asked for unrecognized section name {}",
            sd.name.getLexeme()));
    }
}

void LabelVisitor::enter(const ast::Instruction& lbl, size_t depth) {
    currAddress += 4; // TODO: what about pseudo-ops
}