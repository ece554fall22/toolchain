#include "parser.h"


void Parser::parse() {
    source_file();
}

/*
    source_file ::= unit*
*/
auto Parser::source_file() -> std::unique_ptr<ast::SourceFile> {
    auto n = std::make_unique<ast::SourceFile>();
    while (auto production = unit()) {
        n->units.push_back(std::move(production));
    }
    return n;
}

/*
    unit
        ::= label-decl
        ::= instruction
        ::= macrodef
        ::= macrocall
*/
auto Parser::unit() -> std::unique_ptr<ast::Unit> {
    // TODO
    // auto n = std::make_unique<ast::Unit>();
    // return n;
    return nullptr;
}


/*
    label-decl
        ::= IDENTIFIER COLON
*/
auto Parser::label_decl() -> std::unique_ptr<ast::LabelDecl> {
    auto n = std::make_unique<ast::LabelDecl>();
    // curr().
    return n;
}


/*
    instruction
        ::= IDENTIFIER instruction-args?
*/

/*
    instruction-args
        ::= instruction-arg
        ::= instruction-arg COMMA instruction-args
*/

/*
    instruction-arg
        ::= identifier
        ::= integer
        ::= [ identifier PLUS_EQUALS integer ]
*/
