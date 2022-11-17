#pragma once

#include "ty.h"
#include "util.h"
#include "varint.h"
#include <cstdint>

namespace isa {

struct InstructionVisitor {
    virtual ~InstructionVisitor() = default;

    // misc
    virtual void nop() = 0;
    virtual void halt() = 0;

    // J
    virtual void jmp(s<25> imm) = 0;
    virtual void jal(s<25> imm) = 0;
    // JR
    virtual void jmpr(reg_idx rT, s<20> imm) = 0;
    virtual void jalr(reg_idx rT, s<20> imm) = 0;

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) = 0;
    // BR
    virtual void branchreg(condition_t cond, reg_idx rT, s<17> imm) = 0;
};

struct PrintVisitor : public InstructionVisitor {
    virtual ~PrintVisitor() = default;

    // misc
    virtual void nop() { std::cout << "nop\n"; }
    virtual void halt() { std::cout << "halt\n"; }

    // J
    virtual void jmp(s<25> imm) { std::cout << "jmp " << imm << "\n"; }
    virtual void jal(s<25> imm) { std::cout << "jal " << imm << "\n"; }

    // JR
    virtual void jmpr(reg_idx rT, s<20> imm) {
        std::cout << "jal r" << rT << ", " << imm << "\n";
    }
    virtual void jalr(reg_idx rT, s<20> imm) {
        std::cout << "jal r" << rT << ", " << imm << "\n";
    }

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) {
        std::cout << "branch_imm " << cond << ", " << imm << "\n";
    }
    // BR
    virtual void branchreg(condition_t cond, reg_idx rT, s<17> imm) {
        std::cout << "branch_reg " << cond << ", " << rT << ", " << imm << "\n";
    }
};

void decodeJ(InstructionVisitor& visit, uint32_t instr);
void decodeJR(InstructionVisitor& visit, uint32_t instr);
void decodeBR(InstructionVisitor& visit, uint32_t instr);
void decodeBI(InstructionVisitor& visit, uint32_t instr);
void decodeInstruction(InstructionVisitor& visit, uint32_t instr);

} // namespace isa
