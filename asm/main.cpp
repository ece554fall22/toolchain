#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <fmt/core.h>

#include "emit.h"
#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "symtab.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("asm");

    ap.add_argument("-o", "--output")
        .help("output path")
        .metavar("OUT")
        .default_value(std::string{"out.o"});

    ap.add_argument("--dump-lexemes")
        .help("dump the output of the lexer")
        .default_value(false)
        .implicit_value(true);

    ap.add_argument("--dump-ast")
        .help("dump the AST immediately after parsing")
        .default_value(false)
        .implicit_value(true);

    ap.add_argument("--dump-symtab")
        .help("dump the symbol table")
        .default_value(false)
        .implicit_value(true);

    ap.add_argument("source");

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    auto input = ap.get<std::string>("source");

    std::ifstream f;
    f.open(input);
    if (!f.is_open()) {
        std::cerr << "[!] cant open " << input << std::endl;
        std::exit(1);
    }
    f.seekg(0, std::ios::end);
    size_t fsize = f.tellg();
    std::string sourcebuf(fsize, '\0');
    f.seekg(0);
    f.read(&sourcebuf[0], fsize);

    Lexer lexer(sourcebuf.c_str());

    std::vector<Token> tokens;
    while (true) {
        auto token = lexer.next();
        tokens.push_back(token);
        if (token.isEoF())
            break;
    }

    if (ap["--dump-lexemes"] == true) {
        dumpTokens(tokens);
    }

    Parser parser(tokens);
    parser.parse();

    if (ap["--dump-ast"] == true) {
        ASTPrintVisitor debugVisitor(std::cout);
        parser.visit(debugVisitor);
    }

    SemanticsPass semanticsPass{};
    parser.visit(semanticsPass);
    auto& semaErrors = semanticsPass.getErrors();
    if (!semaErrors.empty()) {
        fmt::print(fmt::emphasis::bold | fmt::emphasis::underline |
                       fmt::fg(fmt::color::red),
                   "errors in semantics pass:\n");

        size_t i = 0;
        for (auto& err : semaErrors) {
            i++;
            fmt::print(fmt::fg(fmt::color::red), "  line {}: {}\n", err.lineno,
                       err.err);
        }

        fmt::print(fmt::fg(fmt::color::red),
                   "\n-- aborting due to errors --\n");
        exit(1);
    }

    LabelVisitor labelPass{};
    parser.visit(labelPass);
    if (ap["--dump-symtab"] == true) {
        std::cout << labelPass.getSymtab() << '\n';
    }

    EmissionPass emissionPass(labelPass.getSymtab());
    parser.visit(emissionPass);

    auto output = emissionPass.getData();
    {
        std::ofstream fOut(ap.get<std::string>("--output"), std::ios::binary);
        for (uint32_t word : output) {
            char v[4];
            v[0] = (word >> 0) & 0xff;
            v[1] = (word >> 8) & 0xff;
            v[2] = (word >> 16) & 0xff;
            v[3] = (word >> 24) & 0xff;
            fOut.write(v, sizeof(v));
        }
    }
}
