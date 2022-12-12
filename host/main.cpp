// >be me
// >clueless
// >working on 639 project cuz i procrastinated it
// >fuck ML fuck pytorch
// >wait
// >554 deadline is tuesday
// >Aware.gif
// >nvm gotta help out the team
// >write code
// >its endian-fragile
// mfw
#include <argparse/argparse.hpp>
#include <csignal>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include "cpu.h"
#include "debugger.h"
#include "iproxy.h"
#include "trace.h"

auto parseArgs(int argc, char* argv[]) -> argparse::ArgumentParser {
    argparse::ArgumentParser ap("host");

    ap.add_argument("codeimage");
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

volatile std::sig_atomic_t signal_flag = 0;
void handle_sigint(int signal) { signal_flag = signal; }

struct Accelerator {
    virtual ~Accelerator() = default;

    virtual void copyToCard(uint32_t* hostSrc, uint64_t cardDst,
                            uint64_t len) = 0;
    virtual void copyFromCard(uint32_t* hostDst, uint64_t cardSrc,
                              uint64_t len) = 0;

    virtual void resetCores(uint64_t cores) = 0;
    virtual void haltCores(uint64_t cores) = 0;
    virtual void unhaltCores(uint64_t cores) = 0;
    virtual auto checkDirty() -> uint64_t = 0;
};

struct SimulatedCard : public Accelerator {
    SimulatedCard(size_t memSize)
        : tracer(std::make_shared<NullTracer>()), cpu{}, mem(memSize, tracer),
          debugger(cpu, mem, quitting), iproxy(cpu, mem, debugger, tracer),
          quitting(false) {}
    virtual ~SimulatedCard() = default;

    std::shared_ptr<Tracer> tracer;
    CPUState cpu;
    MemSystem mem;
    Debugger debugger;
    CPUInstructionProxy iproxy;
    bool quitting;

    void tick() {
        auto pc = cpu.pc.getNewPC();
        auto ir = mem.readInstruction(pc);

        tracer->begin(pc, ir);

        // execute instruction
        isa::decodeInstruction(iproxy, bits<32>(ir));

        tracer->end();
        if (signal_flag == SIGINT) {
            fmt::print(" simulation stopped by SIGINT\n");
            signal_flag = 0;
            debugger.simHaltedByUser();
        }
        debugger.tick();
    }

    // accelerator interface
    void copyToCard(uint32_t* hostSrc, uint64_t cardDst,
                    uint64_t len) override {
        mem._check_addr(cardDst, 32); // align to 32 bit words
        if (len % 4 != 0) {
            std::cerr
                << "copy length must be a multiple of 4 bytes (i.e., copy "
                   "length must be an integer multiple of a 32-bit word)\n";
            std::exit(1);
        }

        memcpy(mem.mempool.data() + cardDst / 4, hostSrc, len); // len in bytes
    }

    void copyFromCard(uint32_t* hostDst, uint64_t cardSrc,
                      uint64_t len) override {
        mem._check_addr(cardSrc, 32); // align to 32 bit words
        if (len % 4 != 0) {
            std::cerr
                << "copy length must be a multiple of 4 bytes (i.e., copy "
                   "length must be an integer multiple of a 32-bit word)\n";
            std::exit(1);
        }

        memcpy(hostDst, mem.mempool.data() + cardSrc / 4, len); // len in bytes
    }

    void resetCores(uint64_t cores) override { unimplemented(); }

    void haltCores(uint64_t cores) override { unimplemented(); }

    void unhaltCores(uint64_t cores) override { unimplemented(); }

    auto checkDirty() -> uint64_t override {
        unimplemented();
        return 0;
    }
};

int main(int argc, char* argv[]) {
    auto ap = parseArgs(argc, argv);

    std::signal(SIGINT, handle_sigint);

    CPUState cpuState;
    size_t memSize = ap.get<size_t>("--mem-size");
    if ((memSize % (128 / 4)) != 0) {
        std::cerr
            << "[!] memory size must be a multiple of 128 bits (16 bytes)\n";
        exit(1);
    }

    SimulatedCard card(memSize);

    // -- load code image
    auto codePath = ap.get<std::string>("codeimage");
    if (!std::filesystem::is_regular_file(codePath)) {
        fmt::print(stderr, "[!] `{}` is not a file\n", codePath);
        exit(1);
    }
    std::ifstream fBin(codePath, std::ios::binary);
    if (!fBin.is_open()) {
        fmt::print(stderr, "[!] can't open `{}`\n", codePath);
        exit(1);
    }
    fBin.seekg(0, std::ios::end);
    size_t fsize = fBin.tellg();
    if (fsize % 4 != 0) {
        fmt::print(stderr,
                   "[!] load a code image thats a multiple of 4 bytes pls\n");
        std::exit(1);
    }
    std::vector<uint32_t> codeImage(fsize / 4);
    fBin.seekg(0);
    fBin.read(reinterpret_cast<char*>(codeImage.data()), fsize);

    // -- copy code image to card
    card.copyToCard(codeImage.data(), 0x0, codeImage.size() * 4);

    // -- run processor
    while (!card.cpu.isHalted()) {
        card.tick();
        if (card.quitting) {
            card.cpu.dump();
            break;
        }
    }

    return 0;
}
