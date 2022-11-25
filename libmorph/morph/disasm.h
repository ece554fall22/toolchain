#pragma once

#include "decoder.h"

// wraps the decoder and provides a structured format for decoded instructions
namespace isa::disasm {
enum class Opcode {
    halt,
    nop,

    jmp,
    jal,
    jmpr,
    jalr,

    bnzi,
    bezi,
    blzi,
    bgzi,
    blei,
    bgei,

    bnzr,
    bezr,
    blzr,
    bgzr,
    bler,
    bger,

    lil,
    lih,

    ld32,
    ld36,
    st32,
    st36,
};
struct Instruction {
    Instruction() = default;

    Opcode opcode;

    // rD rA rB
    reg_idx oper_regs[3];
    int64_t imm;
};

struct DisasmVisitor : public InstructionVisitor {
  private:
    Instruction instr;

  public:
    virtual ~DisasmVisitor() = default;

    auto getInstr() -> Instruction { return instr; }

    // misc
    virtual void nop() { instr.opcode = Opcode::nop; }
    virtual void halt() { instr.opcode = Opcode::halt; }

    // J
    virtual void jmp(s<25> imm) {
        instr.opcode = Opcode::jmp;
        instr.imm = imm._sgn_inner();
    }
    virtual void jal(s<25> imm) {
        instr.opcode = Opcode::jal;
        instr.imm = imm._sgn_inner();
    }

    // JR
    virtual void jmpr(reg_idx rA, s<20> imm) {
        instr.opcode = Opcode::jmpr;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }
    virtual void jalr(reg_idx rA, s<20> imm) {
        instr.opcode = Opcode::jalr;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) {
        switch (cond) {
        case condition_t::nz:
            instr.opcode = Opcode::bnzi;
            break;
        case condition_t::ez:
            instr.opcode = Opcode::bezi;
            break;
        case condition_t::lz:
            instr.opcode = Opcode::blzi;
            break;
        case condition_t::gz:
            instr.opcode = Opcode::bgzi;
            break;
        case condition_t::le:
            instr.opcode = Opcode::blei;
            break;
        case condition_t::ge:
            instr.opcode = Opcode::bgei;
            break;
        }

        instr.imm = imm._sgn_inner();
    }
    // BR
    virtual void branchreg(condition_t cond, reg_idx rA, s<17> imm) {
        switch (cond) {
        case condition_t::nz:
            instr.opcode = Opcode::bnzr;
            break;
        case condition_t::ez:
            instr.opcode = Opcode::bezr;
            break;
        case condition_t::lz:
            instr.opcode = Opcode::blzr;
            break;
        case condition_t::gz:
            instr.opcode = Opcode::bgzr;
            break;
        case condition_t::le:
            instr.opcode = Opcode::bler;
            break;
        case condition_t::ge:
            instr.opcode = Opcode::bger;
            break;
        }

        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }

    virtual void lil(reg_idx rD, s<18> imm) {
        instr.opcode = Opcode::lil;
        instr.oper_regs[0] = rD;
        instr.imm = imm._sgn_inner();
    }
    virtual void lih(reg_idx rD, s<18> imm) {
        instr.opcode = Opcode::lih;
        instr.oper_regs[0] = rD;
        instr.imm = imm._sgn_inner();
    }

    virtual void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) {
        if (b36)
            instr.opcode = Opcode::ld36;
        else
            instr.opcode = Opcode::ld32;

        instr.oper_regs[0] = rD;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }

    virtual void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) {
        if (b36)
            instr.opcode = Opcode::st36;
        else
            instr.opcode = Opcode::st32;

        instr.oper_regs[1] = rA;
        instr.oper_regs[2] = rB;
        instr.imm = imm._sgn_inner();
    }
};

}; // namespace isa::disasm
