#pragma once

#include "ty.h"
#include "util.h"
#include "varint.h"
#include <cstdint>

namespace isa {

namespace disasm {
enum class Opcode {
    halt,
    nop,
    jmp,
    jal,
    jmpr,
    jalr,
};
struct Instruction {
    Opcode opcode;

    reg_idx register_accesses[3];
    int64_t imm;
};
} // namespace disasm

struct InstructionVisitor {
    virtual ~InstructionVisitor() = default;

    // misc
    virtual void nop() = 0;
    virtual void halt() = 0;

    // J
    virtual void jmp(s<25> imm) = 0;
    virtual void jal(s<25> imm) = 0;
    // JR
    virtual void jmpr(reg_idx rA, s<20> imm) = 0;
    virtual void jalr(reg_idx rA, s<20> imm) = 0;

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) = 0;
    // BR
    virtual void branchreg(condition_t cond, reg_idx rA, s<17> imm) = 0;

    // LI
    virtual void lil(reg_idx rD, s<18> imm) = 0;
    virtual void lih(reg_idx rD, s<18> imm) = 0;

    // ML
    virtual void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) = 0;
    // MS
    virtual void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) = 0;
};

void decodeInstruction(InstructionVisitor& visit, bits<32> instr);

struct PrintVisitor : public InstructionVisitor {
    virtual ~PrintVisitor() = default;

    // misc
    virtual void nop() { std::cout << "nop\n"; }
    virtual void halt() { std::cout << "halt\n"; }

    // J
    virtual void jmp(s<25> imm) { std::cout << "jmp " << imm << "\n"; }
    virtual void jal(s<25> imm) { std::cout << "jal " << imm << "\n"; }

    // JR
    virtual void jmpr(reg_idx rA, s<20> imm) {
        std::cout << "jal r" << rA << ", " << imm << "\n";
    }
    virtual void jalr(reg_idx rA, s<20> imm) {
        std::cout << "jal r" << rA << ", " << imm << "\n";
    }

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) {
        std::cout << "branch_imm " << cond << ", " << imm << "\n";
    }
    // BR
    virtual void branchreg(condition_t cond, reg_idx rA, s<17> imm) {
        std::cout << "branch_reg " << cond << ", " << rA << ", " << imm << "\n";
    }

    virtual void lil(reg_idx rD, s<18> imm) {
        std::cout << "lil " << rD << ", " << imm << "\n";
    }
    virtual void lih(reg_idx rD, s<18> imm) {
        std::cout << "lih " << rD << ", " << imm << "\n";
    }

    virtual void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) {
        std::cout << "ld";
        if (b36)
            std::cout << "36";
        else
            std::cout << "32";
        std::cout << " " << rD << ", " << rA << ", " << imm << '\n';
    }

    virtual void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) {
        std::cout << "st";
        if (b36)
            std::cout << "36";
        else
            std::cout << "32";
        std::cout << " " << rA << ", " << rB << ", " << imm << '\n';
    }
};
} // namespace isa
