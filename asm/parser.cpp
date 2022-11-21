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

    // std::cout << "  we are at " << curr().getKind() << "\n";
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

    // eat :
    next(2);

    return n;
}

/*
    instruction
        ::= IDENTIFIER operands? LINEBREAK

    operands
        ::= operand
        ::= operand COMMA operands
*/
auto Parser::instruction() -> std::unique_ptr<ast::Instruction> {
    if (curr().isNot(Token::Kind::IDENTIFIER))
        return nullptr;

    // IDENTIFIER
    auto mnemonic = curr();

    // operands?
    next();
    std::vector<ast::Operand> args;
    while (auto arg = operand()) {
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
        error(fmt::format("expected linebreak after instruction, not {}",
                          curr().getKind()));
        return nullptr;
    }

    next();

    return std::make_unique<ast::Instruction>(mnemonic, std::move(args));
}

/*
    operand
        ::= identifier
        ::= integer
        ::= operand-memory
*/
auto Parser::operand() -> std::optional<ast::Operand> {
    if (curr().is(Token::Kind::IDENTIFIER)) {
        auto n = ast::Operand{ast::OperandIdentifier{curr()}};
        next();
        return n;
    } else if (curr().isIntegerLiteral()) {
        auto n = ast::Operand{ast::OperandImmediate{69}}; // TODO FIXME
        next();
        return n;
    } else if (auto a = operand_memory()) {
        return ast::Operand{*a};
    } else {
        return std::nullopt;
    }
}

/*
    operand-memory
        ::= L_SQUARE identifier R_SQUARE
        ::= L_SQUARE identifier PLUS integer R_SQUARE
    // nb: only for vector incr loads
        ::= L_SQUARE identifier PLUS_EQUALS integer R_SQUARE
    // nb: ibid
        ::= L_SQUARE identifier PLUS_EQUALS identifier R_SQUARE
*/
auto Parser::operand_memory() -> std::optional<ast::OperandMemory> {
    if (curr().isNot(Token::Kind::L_SQUARE))
        return std::nullopt;

    auto base = next();
    if (base.isNot(Token::Kind::IDENTIFIER)) {
        error("expected an identifier following [ in an operand addressing "
              "memory");
        return std::nullopt;
    }

    next();
    if (curr().is(Token::Kind::R_SQUARE)) { // early ending!
        next();                             // eat ]
        return ast::OperandMemory{base, 0, false};
    } else if (curr().is(Token::Kind::PLUS)) { // offset
        auto offset = next();
        if (!offset.isIntegerLiteral()) {
            error(fmt::format("offset in memory addressing operand must be an "
                              "integer, not {}",
                              curr().getKind()));
            return std::nullopt;
        }
        auto offsetVal = parseIntegerToken(offset);
        if (!offsetVal) {
            error(fmt::format(
                "can't parse {} (`{}`) as integer literal in memory offset",
                offset.getKind(), offset.getLexeme()));
            return std::nullopt;
        }

        if (next().isNot(Token::Kind::R_SQUARE)) {
            error(fmt::format("memory operand must end with a ], not {}",
                              curr().getKind()));
            return std::nullopt;
        }

        next(); // eat ]

        return ast::OperandMemory{base, *offsetVal, false};
    } else if (curr().is(Token::Kind::PLUSEQUAL)) { // offset and postincrement
        auto offset = next();
        if (!offset.isIntegerLiteral() &&
            offset.isNot(Token::Kind::IDENTIFIER)) {
            error(fmt::format("offset in memory addressing operand must be an "
                              "integer or register, not {}",
                              curr().getKind()));
            return std::nullopt;
        }
        auto offsetVal = parseIntegerToken(offset);
        if (!offsetVal) {
            error(fmt::format(
                "can't parse {} (`{}`) as integer literal in memory offset",
                offset.getKind(), offset.getLexeme()));
            return std::nullopt;
        }

        if (next().isNot(Token::Kind::R_SQUARE)) {
            error(fmt::format("memory operand must end with a ], not {}",
                              curr().getKind()));
            return std::nullopt;
        }

        next(); // eat ]

        return ast::OperandMemory{base, *offsetVal, true};
    } else {
        error(fmt::format("unexpected token {}", curr().getKind()));
        return std::nullopt;
    }
}
