#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <csignal>

#include "cpu.h"
#include "instructions.h"
#include "trace.h"

#include <morph/decoder.h>
#include <morph/util.h>
#include <fmt/color.h>

class CPUInstructionProxy : public isa::InstructionVisitor {
public:
    ~CPUInstructionProxy() override = default;
    CPUInstructionProxy(auto& cpu, auto& mem) : cpu{cpu}, mem{mem} {}
    CPUInstructionProxy(auto&& cpu, auto&& mem) = delete;
    // misc
    void nop() override { instructions::nop(cpu, mem); }
    void halt() override { instructions::halt(cpu, mem); }
    void bkpt(bits<25> signal) override {
        fmt::print(
            "\n\nBREAKPOINT BREAKPOINT : {:#x} : BREAKPOINT BREAKPOINT\n\n",
            signal.inner);
    }

    // J
    void jmp(s<25> imm) override { instructions::jmp(cpu, mem, imm); }
    void jal(s<25> imm) override { instructions::jal(cpu, mem, imm); }

    // JR
    void jmpr(reg_idx rA, s<20> imm) override {
        instructions::jmpr(cpu, mem, rA, imm);
    }
    void jalr(reg_idx rA, s<20> imm) override {
        instructions::jalr(cpu, mem, rA, imm);
    }

    // BI
    void branchimm(condition_t cond, s<22> imm) override {
        todo("branch instruction proxies");
    }
    // BR
    void branchreg(condition_t cond, reg_idx rA, s<17> imm) override {
        todo("branch instruction proxies");
    }

    void lil(reg_idx rD, s<18> imm) override {
        instructions::lil(cpu, mem, rD, imm);
    }
    void lih(reg_idx rD, s<18> imm) override {
        instructions::lih(cpu, mem, rD, imm);
    }

    void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) override {
        if (b36)
            instructions::ld36(cpu, mem, rD, rA, imm);
        else
            instructions::ld32(cpu, mem, rD, rA, imm);
    }

    void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) override {
        if (b36)
            instructions::st36(cpu, mem, rA, rB, imm);
        else
            instructions::st32(cpu, mem, rA, rB, imm);
    }

    void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                                  isa::ScalarArithmeticOp op) override {
        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instructions::add(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Sub:
            instructions::sub(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Mul:
            instructions::mul(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::And:
            instructions::and_(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Or:
            instructions::or_(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Xor:
            instructions::xor_(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Shr:
            instructions::shr(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Shl:
            instructions::shl(cpu, mem, rD, rA, rB);
            return;
        }
    }

    void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                           isa::ScalarArithmeticOp op) override {
        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instructions::addi(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Sub:
            instructions::subi(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::And:
            instructions::andi(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Or:
            instructions::ori(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Xor:
            instructions::xori(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Shr:
            instructions::shri(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Shl:
            instructions::shli(cpu, mem, rD, rA, imm);
            return;
        default:
            panic("invalid op for immedate");
        }
    }

  private:
    CPUState& cpu;
    MemSystem& mem;
};

volatile std::sig_atomic_t signal_flag = 0;
void handle_sigint(int signal) {
    signal_flag = signal;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("asm");

    ap.add_argument("memory");

    ap.add_argument("--trace").help("write a tracefile").metavar("TRACEFILE");

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    std::signal(SIGINT, handle_sigint);

    std::unique_ptr<Tracer> tracer = nullptr;
    if (auto tracepath = ap.present<std::string>("--trace")) {
        tracer = make_unique<Tracer>(*tracepath);
    }

    CPUState cpuState(std::move(tracer));
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
        // WARNING WARNING TODO(erin): only works on little-endian architectures
        fBin.read(reinterpret_cast<char*>(mem.mempool.data()),
                  mem.mempool.size() * 4);
    }

    fmt::print("dumping 0 page:\n");
    for (size_t i = 0; i < 32; i++) {
        fmt::print("{:#x}: {:#x}\n", i, mem.mempool[i]);
    }
    fmt::print("\n");

    while (!cpuState.isHalted()) {
        auto pc = cpuState.pc.getNewPC();
        auto ir = mem.readInstruction(pc);

        if (cpuState.tracer) cpuState.tracer->begin(pc, ir);

        // TODO(erin): temp stdout logging
        fmt::print("pc={:#x} ir={:#x}\n", pc, ir);
        std::cout << "] ";
        isa::decodeInstruction(printvis, bits<32>(ir));

        // execute instruction
        isa::decodeInstruction(iproxy, bits<32>(ir));

        if (cpuState.tracer) cpuState.tracer->end();

        if (signal_flag == SIGINT) {
            fmt::print(fmt::fg(fmt::color::cyan), " simulation halted by SIGINT\n");
            cpuState.halt();
        }
    }

    cpuState.dump();
}
