#include "parser.h"

void Parser::parse() {
    astRoot = source_file();

    if (!eof()) {
        // error("unexpected trailing content after parser finished");
    }
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
        ::= macro-invoke
        ::= macro-def
*/
auto Parser::unit() -> std::unique_ptr<ast::Unit> {
    // TODO
    // auto n = std::make_unique<ast::Unit>();
    // return n;
    // return nullptr;
    // if (auto ld = label_decl()) {

    // } else if ()

    // eat up any linebreaks
    while (curr().is(Token::Kind::LINEBREAK)) {
        next();
    }

    if (auto l = label_decl()) {
        return std::make_unique<ast::Unit>(std::move(l));
    } else if (auto i = instruction()) {
        return std::make_unique<ast::Unit>(std::move(i));
    }

    return nullptr;
}

/*
    label-decl
        ::= IDENTIFIER COLON
*/
auto Parser::label_decl() -> std::unique_ptr<ast::LabelDecl> {
    if (curr().isNot(Token::Kind::IDENTIFIER))
        return nullptr;
    // lookahead to fail out early instead of rewinding
    if (peek().isNot(Token::Kind::COLON))
        return nullptr;

    auto ident = curr();
    auto n = std::make_unique<ast::LabelDecl>(ident);

    return n;
}

/*
    instruction
        ::= IDENTIFIER instruction-args? LINEBREAK

    instruction-args
        ::= instruction-arg
        ::= instruction-arg COMMA instruction-args
*/
auto Parser::instruction() -> std::unique_ptr<ast::Instruction> {
    if (curr().isNot(Token::Kind::IDENTIFIER))
        return nullptr;

    // IDENTIFIER
    auto mnemonic = curr();

    // instruction-args?
    next();
    std::vector<ast::InstructionArg> args;
    while (auto arg = instruction_arg()) {
        args.push_back(*arg);
        if (curr().is(Token::Kind::LINEBREAK)) {
            break;
        } else if (curr().isNot(Token::Kind::COMMA)) {
            error("expected , in instruction argument list");
            return nullptr;
        }

        next(); // eat ,
    }

    // LINEBREAK
    if (curr().isNot(Token::Kind::LINEBREAK)) {
        error("expected linebreak after instruction");
        return nullptr;
    }

    return std::make_unique<ast::Instruction>(mnemonic, std::move(args));
}

/*
    instruction-arg
        ::= identifier
        ::= integer
        ::= addressing

    addressing
        ::= L_SQUARE identifier R_SQUARE
        ::= L_SQUARE identifier PLUS integer R_SQUARE
        ::= L_SQUARE identifier PLUS_EQUALS integer R_SQUARE     // nb: only for
   vector incr loads
        ::= L_SQUARE identifier PLUS_EQUALS identifier R_SQUARE  // nb: ibid
*/
auto Parser::instruction_arg() -> std::optional<ast::InstructionArg> {
    if (curr().is(Token::Kind::IDENTIFIER)) {
        auto n = ast::InstructionArg{curr()};
        next();
        return n;
    } else if (curr().is(Token::Kind::INTEGER)) {
        auto n = ast::InstructionArg{curr()};
        next();
        return n;
    } else {
        return std::nullopt;
    }
}

// auto Parser::instruction_args() -> std::unique_ptr<ast::InstructionArgs> {
//     return nullptr;
// }
