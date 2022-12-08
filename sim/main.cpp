#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <Eigen/Dense>
#include <argparse/argparse.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include <morph/decoder.h>
#include <morph/util.h>

#include "cpu.h"
#include "debugger.h"
#include "iproxy.h"

using json = nlohmann::json;

volatile std::sig_atomic_t signal_flag = 0;
void handle_sigint(int signal) { signal_flag = signal; }

auto parseArgs(int argc, char* argv[]) -> argparse::ArgumentParser {
    argparse::ArgumentParser ap("sim");

    ap.add_argument("memory");

    ap.add_argument("--trace").help("write a tracefile").metavar("TRACEFILE");
    ap.add_argument("--init-state")
        .help("seed the CPU state from a JSON file")
        .metavar("SEED");
    ap.add_argument("--log-execution")
        .help("print PC, IR, and disassembly for each executed instruction "
              "(NOT a formal trace format!)")
        .default_value(false)
        .implicit_value(true);
    ap.add_argument("--mem-size")
        .help("size of emulated memory space, as # of 32-bit words. must be a "
              "multiple of 128 bits. default is 1MiB")
        .default_value<size_t>((1 << 20) / 4) // 1MiB
        .scan<'d', size_t>();

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    return ap;
}

void loadMemoryImage(MemSystem& dest, const std::string& path) {
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
    fBin.read(reinterpret_cast<char*>(dest.mempool.data()),
              dest.mempool.size() * 4);
}

void initState(CPUState& cpuState, const std::string& path) {
    if (!std::filesystem::is_regular_file(path)) {
        fmt::print(stderr, "[!] `{}` is not a file\n", path);
        exit(1);
    }

    std::ifstream fInit(path);
    if (!fInit.is_open()) {
        fmt::print(stderr, "[!] can't open `{}`\n", path);
        exit(1);
    }

    json data = json::parse(fInit, nullptr, true, true);
    for (auto& [cpukey, cpudata] : data.at("cpus").items()) {
        size_t cpu_idx = std::stoi(cpukey);
        assert(cpu_idx == 0 && "lol one cpu"); // todo

        if (cpudata.contains("r")) {
            auto regvals = cpudata["r"];
            assert(regvals.is_array() && "cpu/{n}/r must be array");
            assert(regvals.size() == 32 && "cpu/{n}/r must be 32 el long");
            size_t reg_idx = 0;
            for (auto& regval : regvals) {
                cpuState.r[reg_idx] = regval.get<int64_t>();
                reg_idx++;
            }
        }

        if (cpudata.contains("v")) {
            auto vecvals = cpudata["v"];
            assert(vecvals.is_array() && "cpu/{n}/v must be array");
            assert(vecvals.size() == 32 && "cpu/{n}/v must be 32 el long");
            size_t reg_idx = 0;
            for (auto& vec : vecvals) {
                for (size_t lane = 0; lane < N_LANES; lane++) {
                    cpuState.v[reg_idx][lane] = vec.at(lane).get<float>();
                }
                reg_idx++;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    auto ap = parseArgs(argc, argv);

    std::signal(SIGINT, handle_sigint);

    std::shared_ptr<Tracer> tracer = nullptr;
    if (auto tracepath = ap.present<std::string>("--trace")) {
        tracer = std::make_shared<FileTracer>(*tracepath);
    } else {
        tracer = std::make_shared<NullTracer>();
    }

    CPUState cpuState;
    size_t memSize = ap.get<size_t>("--mem-size");
    if ((memSize % (128 / 4)) != 0) {
        std::cerr
            << "[!] memory size must be a multiple of 128 bits (16 bytes)\n";
        exit(1);
    }
    MemSystem mem(memSize, tracer);
    Debugger debugger(cpuState, mem);
    CPUInstructionProxy iproxy(cpuState, mem, debugger, tracer);
    isa::PrintVisitor printvis(std::cout);

    loadMemoryImage(mem, ap.get<std::string>("memory"));

    if (auto path = ap.present<std::string>("--init-state")) {
        initState(cpuState, *path);
    }

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
        debugger.tick();

        if (signal_flag == SIGINT) {
            fmt::print(fmt::fg(fmt::color::cyan),
                       " simulation halted by SIGINT\n");
            cpuState.halt();
        }
    }

    cpuState.dump();
}
