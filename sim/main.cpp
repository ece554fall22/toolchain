#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>

#include "cpu.h"
#include "instructions.h"

#include <morph/decoder.h>
#include <morph/util.h>

// #define PROXY_INSTR(instr, ...) \
//     virtual void instr (__VA_ARGS__) { \
//         instructions:: instr ( this->cpu, this->mem, ## __VA_ARGS__ ); \
//     }

struct CPUInstructionProxy : public isa::InstructionVisitor {
    virtual ~CPUInstructionProxy() = default;
    CPUInstructionProxy(auto& cpu, auto& mem) : cpu{cpu}, mem{mem} {}
    CPUInstructionProxy(auto&& cpu, auto&& mem) = delete;
    // misc
    virtual void nop() { instructions::nop(cpu, mem); }
    virtual void halt() { instructions::halt(cpu, mem); }
    virtual void bkpt(bits<25> signal) {
        fmt::print(
            "\n\nBREAKPOINT BREAKPOINT : {:#x} : BREAKPOINT BREAKPOINT\n\n",
            signal.inner);
    }

    // J
    virtual void jmp(s<25> imm) { instructions::jmp(cpu, mem, imm); }
    virtual void jal(s<25> imm) { instructions::jal(cpu, mem, imm); }

    // JR
    virtual void jmpr(reg_idx rA, s<20> imm) {
        instructions::jmpr(cpu, mem, rA, imm);
    }
    virtual void jalr(reg_idx rA, s<20> imm) {
        instructions::jalr(cpu, mem, rA, imm);
    }

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) {
        todo("branch instruction proxies");
    }
    // BR
    virtual void branchreg(condition_t cond, reg_idx rA, s<17> imm) {
        todo("branch instruction proxies");
    }

    virtual void lil(reg_idx rD, s<18> imm) {
        instructions::lil(cpu, mem, rD, imm);
    }
    virtual void lih(reg_idx rD, s<18> imm) {
        instructions::lih(cpu, mem, rD, imm);
    }

    virtual void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) {
        if (b36)
            instructions::ld36(cpu, mem, rD, rA, imm);
        else
            instructions::ld32(cpu, mem, rD, rA, imm);
    }

    virtual void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) {
        if (b36)
            instructions::st36(cpu, mem, rA, rB, imm);
        else
            instructions::st32(cpu, mem, rA, rB, imm);
    }

    virtual void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                                  isa::ScalarArithmeticOp op) {
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

    virtual void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                           isa::ScalarArithmeticOp op) {
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

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("asm");

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
    CPUInstructionProxy iproxy{cpuState, mem};
    isa::PrintVisitor printvis;

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
