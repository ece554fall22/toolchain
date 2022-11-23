#include <argparse/argparse.hpp>
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

    // misc
    virtual void nop() { instructions::nop(cpu, mem); }
    virtual void halt() { instructions::halt(cpu, mem); }

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

  private:
    CPUState& cpu;
    MemSystem& mem;
};

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
    CPUInstructionProxy iproxy{cpuState, mem};

    // // "run" a little program
    // instructions::addi(cpuState, mem, /*r*/ 0, /*r*/ 0, 1);

    cpuState.dump();
}
