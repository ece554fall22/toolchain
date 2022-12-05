#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <argparse/argparse.hpp>
#include <fmt/color.h>
#include <fmt/core.h>

#include <morph/decoder.h>
#include <morph/util.h>

#include "cpu.h"
#include "iproxy.h"

volatile std::sig_atomic_t signal_flag = 0;
void handle_sigint(int signal) { signal_flag = signal; }

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("sim");

    ap.add_argument("memory");

    ap.add_argument("--trace").help("write a tracefile").metavar("TRACEFILE");

    ap.add_argument("--log-execution")
        .help("print PC, IR, and disassembly for each executed instruction (NOT a formal trace format!)")
        .default_value(false)
        .implicit_value(true);

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    std::signal(SIGINT, handle_sigint);

    std::shared_ptr<Tracer> tracer = nullptr;
    if (auto tracepath = ap.present<std::string>("--trace")) {
        tracer = std::make_shared<FileTracer>(*tracepath);
    } else {
        tracer = std::make_shared<NullTracer>();
    }

    CPUState cpuState;
    MemSystem mem(1024 /* 1k */);
    CPUInstructionProxy iproxy(cpuState, mem, tracer);
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
        // WARNING WARNING TODO(erin): only works on little-endian architectures
        fBin.read(reinterpret_cast<char*>(mem.mempool.data()),
                  mem.mempool.size() * 4);
    }

    // cpuState.v[1][0] = 0.5f;
    // cpuState.v[1][1] = 0.1f;
    // cpuState.v[1][2] = 0.2f;
    // cpuState.v[1][3] = 0.3f;

    // cpuState.v[2][0] = -1.5f;
    // cpuState.v[2][1] = -5.1f;
    // cpuState.v[2][2] = 2.2f;
    // cpuState.v[2][3] = 3.3f;

    // fmt::print("dumping 0 page:\n");
    // for (size_t i = 0; i < 32; i++) {
    //     fmt::print("{:#x}: {:#x}\n", i, mem.mempool[i]);
    // }
    // fmt::print("\n");

    while (!cpuState.isHalted()) {
        auto pc = cpuState.pc.getNewPC();
        auto ir = mem.readInstruction(pc);

        tracer->begin(pc, ir);

        if (ap["--log-execution"] == true) {
            fmt::print("pc={:#x} ir={:#x}\n", pc, ir);
            std::cout << "] ";
            isa::decodeInstruction(printvis, bits<32>(ir));
            std::cout << '\n';
        }

        // execute instruction
        isa::decodeInstruction(iproxy, bits<32>(ir));

        tracer->end();

        if (signal_flag == SIGINT) {
            fmt::print(fmt::fg(fmt::color::cyan),
                       " simulation halted by SIGINT\n");
            cpuState.halt();
        }
    }

    cpuState.dump();
}
