#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>

#include <morph/decoder.h>
#include <morph/util.h>

#include "cpu.h"
#include "iproxy.h"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("sim");

    ap.add_argument("memory");

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    CPUState cpuState;
    MemSystem mem(1024 /* 1k */);
    CPUInstructionProxy iproxy(cpuState, mem);
    isa::PrintVisitor printvis(std::cout);

    {
        auto path = ap.get<std::string>("memory");
        if (!std::filesystem::is_regular_file(path)) {
            fmt::print(stderr, "[!] `{}` is not a file\n", path);
            exit(1);
        }
        std::ifstream fBin(path, std::ios::binary);
        if (!fBin.is_open()) {
            fmt::print(stderr, "[!] can't open `{}`\n", path);
            exit(1);
        }
        // load blocks. WARNING WARNING TODO(erin): only works on little-endian
        // architectures
        fBin.read(reinterpret_cast<char*>(mem.mempool.data()),
                  mem.mempool.size() * 4);
    }

    fmt::print("dumping 0 page:\n");
    for (size_t i = 0; i < 32; i++) {
        fmt::print("{:#x}: {:#x}\n", i, mem.mempool[i]);
    }

    while (!cpuState.isHalted()) {
        auto pc = cpuState.pc.getNewPC();
        auto ir = mem.readInstruction(pc);

        fmt::print("pc={:#x} ir={:#x}\n", pc, ir);
        std::cout << "] ";
        isa::decodeInstruction(printvis, bits<32>(ir));
        isa::decodeInstruction(iproxy, bits<32>(ir));
    }

    // // "run" a little program
    // instructions::addi(cpuState, mem, /*r*/ 0, /*r*/ 0, 1);

    cpuState.dump();
}
