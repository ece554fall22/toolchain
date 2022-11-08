#include <argparse/argparse.hpp>
#include <iostream>

#include "cpu.h"
#include "instructions.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("asm");

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    CPUState cpuState;
    MemSystem mem;

    // "run" a little program
    instructions::addi(cpuState, mem, /*r*/ 0, /*r*/ 0, 1);

    cpuState.dump();
}
