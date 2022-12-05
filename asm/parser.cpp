#include "parser.h"

#include <cctype>
#include <charconv>

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
        ::= directive
*/
auto Parser::unit() -> std::unique_ptr<ast::Unit> {
    // eat up any linebreaks
    while (curr().is(Token::Kind::LINEBREAK)) {
        next();
    }

    // check if we're done
    if (curr().isEoF()) {
        return nullptr;
    }

    // std::cout << "  we are at " << curr().getKind() << "\n";
    if (auto l = label_decl()) {
        return std::make_unique<ast::Unit>(std::move(l));
    } else if (auto i = instruction()) {
        return std::make_unique<ast::Unit>(std::move(i));
    } else if (auto d = directive_origin()) {
        return std::make_unique<ast::Unit>(std::move(d));
    } else if (auto d = directive_section()) {
        return std::make_unique<ast::Unit>(std::move(d));
    } else {
        error(fmt::format("unknown construct beginning with {} (`{}`)",
                          curr().getKind(), curr().getLexeme()));
        return nullptr;
    }
}

/* directive-origin
        ::= % 'org'
*/
auto Parser::directive_origin() -> std::unique_ptr<ast::OriginDirective> {
    if (curr().isNot(Token::Kind::PERCENT))
        return nullptr;

    auto directive = peek();
    if (directive.isNot(Token::Kind::IDENTIFIER) ||
        std::string(directive.getLexeme()) != "org") {
        return nullptr;
    }

    next(); // eat `org`

    auto origin = next();
    if (!origin.isIntegerLiteral()) {
        error("%org must be followed by an integer literal");
        return nullptr;
    }

    auto originVal = parseIntegerToken(origin);
    if (!originVal) {
        error(fmt::format(
            "can't parse {} (`{}`) as integer literal in %org directive",
            origin.getKind(), origin.getLexeme()));
        return nullptr;
    }

    next(); // eat integer

    return std::make_unique<ast::OriginDirective>(*originVal);
}

/* directive-section
        ::= % 'org'
*/
auto Parser::directive_section() -> std::unique_ptr<ast::SectionDirective> {
    if (curr().isNot(Token::Kind::PERCENT))
        return nullptr;

    auto directive = peek();
    if (directive.isNot(Token::Kind::IDENTIFIER) ||
        std::string(directive.getLexeme()) != "section") {
        return nullptr;
    }

    next(); // eat `section`

    auto name = next();
    if (name.isNot(Token::Kind::IDENTIFIER)) {
        error("%section must be followed by a section name");
        return nullptr;
    }

    next(); // eat ident

    return std::make_unique<ast::SectionDirective>(name);
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
        if (curr().isEndOfLine()) {
            break;
        } else if (curr().isNot(Token::Kind::COMMA)) {
            error(fmt::format("expected , in instruction argument list, not {}", curr().getKind()));
            return nullptr;
        }

        next(); // eat ,
    }

    // LINEBREAK
    if (!curr().isEndOfLine()) {
        error(fmt::format("expected linebreak after instruction, not {}",
                          curr().getKind()));
        return nullptr;
    }

    next();

    return std::make_unique<ast::Instruction>(mnemonic, std::move(args));
}

/*
    operand
        ::= register
        ::= label
        ::= integer
        ::= operand-memory
*/
auto Parser::operand() -> std::optional<ast::Operand> {
    if (auto reg = operand_register()) {
        return ast::Operand{*reg};
    } else if (auto lbl = operand_label()) {
        return ast::Operand{*lbl};
    } else if (curr().isIntegerLiteral()) {
        auto val = parseIntegerToken(curr());
        if (!val) {
            error(fmt::format(
                "can't parse {} (`{}`) as integer literal in operand",
                curr().getKind(), curr().getLexeme()));
            return std::nullopt;
        }

        next();
        return ast::Operand{ast::OperandImmediate{*val}};
    } else if (auto a = operand_memory()) {
        return ast::Operand{*a};
    } else {
        return std::nullopt;
    }
}

auto Parser::operand_register() -> std::optional<ast::OperandRegister> {
    if (!curr().is(Token::Kind::IDENTIFIER))
        return std::nullopt;

    auto ident = curr();
    auto sp = ident.getLexeme();
    if (sp.size() >= 2 && isdigit(sp[1]) && (sp[0] == 'r' || sp[0] == 'v')) {
        bool vector = sp[0] == 'v';

        sp.remove_prefix(1);
        uint idx;
        auto res = std::from_chars(sp.data(), sp.data() + sp.size(), idx, 10);
        if (res.ec != std::errc{} || res.ptr != (sp.data() + sp.size())) {
            // TODO? more error handling here?
            return std::nullopt;
        }

        next();
        return ast::OperandRegister{ident, vector, idx};
    } else {
        return std::nullopt;
    }
}

auto Parser::operand_label() -> std::optional<ast::OperandLabel> {
    if (!curr().is(Token::Kind::IDENTIFIER))
        return std::nullopt;

    auto lbl = curr();
    next();
    return ast::OperandLabel{lbl};
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
    next(); // eat [

    auto base = operand_register();
    if (!base) {
        error("expected a register in memory operand");
        return std::nullopt;
    }

    if (curr().is(Token::Kind::R_SQUARE)) { // early ending!
        next();                             // eat ]
        return ast::OperandMemory{*base, 0, false};
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

        return ast::OperandMemory{*base, *offsetVal, false};
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

        return ast::OperandMemory{*base, *offsetVal, true};
    } else {
        error(fmt::format("unexpected token {}", curr().getKind()));
        return std::nullopt;
    }
}

/*
    directive needs:
        origin
        section
        define
*/
// auto Parser::directive() {}
