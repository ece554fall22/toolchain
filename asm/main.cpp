#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <fmt/core.h>

#include "lexer.h"
#include "parser.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("asm");

    ap.add_argument("-o", "--output")
        .help("output path")
        .default_value(std::string{"out.o"});

    ap.add_argument("sources").nargs(argparse::nargs_pattern::at_least_one);

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    auto inputs = ap.get<std::vector<std::string>>("sources");

    std::ifstream f;
    f.open(inputs[0]);
    if (!f.is_open()) {
        std::cerr << "[!] cant open " << inputs[0] << std::endl;
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

    dumpTokens(tokens);

    Parser parser(tokens);
    parser.parse();

    ASTPrintVisitor debugVisitor(std::cout);
    parser.visit(debugVisitor);
}
