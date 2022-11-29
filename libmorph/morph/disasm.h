#pragma once

#include "decoder.h"

// wraps the decoder and provides a structured format for decoded instructions
namespace isa::disasm {
enum class Opcode {
    Halt,
    Nop,
    Bkpt,

    Jmp,
    Jal,
    Jmpr,
    Jalr,

    Bnzi,
    Bezi,
    Blzi,
    Bgzi,
    Blei,
    Bgei,

    Bnzr,
    Bezr,
    Blzr,
    Bgzr,
    Bler,
    Bger,

    Lil,
    Lih,

    Ld32,
    Ld36,
    St32,
    St36,

    Addi,
    Subi,
    Andi,
    Ori,
    Xori,
    Shri,
    Shli,

    Add,
    Sub,
    Mul,
    And,
    Or,
    Xor,
    Shr,
    Shl,
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
    virtual void nop() { instr.opcode = Opcode::Nop; }
    virtual void halt() { instr.opcode = Opcode::Halt; }
    virtual void bkpt(bits<25> signal) {
        instr.opcode = Opcode::Bkpt;
        instr.imm = signal.inner;
    }

    // J
    virtual void jmp(s<25> imm) {
        instr.opcode = Opcode::Jmp;
        instr.imm = imm._sgn_inner();
    }
    virtual void jal(s<25> imm) {
        instr.opcode = Opcode::Jal;
        instr.imm = imm._sgn_inner();
    }

    // JR
    virtual void jmpr(reg_idx rA, s<20> imm) {
        instr.opcode = Opcode::Jmpr;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }
    virtual void jalr(reg_idx rA, s<20> imm) {
        instr.opcode = Opcode::Jalr;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }

    // BI
    virtual void branchimm(condition_t cond, s<22> imm) {
        switch (cond) {
        case condition_t::nz:
            instr.opcode = Opcode::Bnzi;
            break;
        case condition_t::ez:
            instr.opcode = Opcode::Bezi;
            break;
        case condition_t::lz:
            instr.opcode = Opcode::Blzi;
            break;
        case condition_t::gz:
            instr.opcode = Opcode::Bgzi;
            break;
        case condition_t::le:
            instr.opcode = Opcode::Blei;
            break;
        case condition_t::ge:
            instr.opcode = Opcode::Bgei;
            break;
        }

        instr.imm = imm._sgn_inner();
    }
    // BR
    virtual void branchreg(condition_t cond, reg_idx rA, s<17> imm) {
        switch (cond) {
        case condition_t::nz:
            instr.opcode = Opcode::Bnzr;
            break;
        case condition_t::ez:
            instr.opcode = Opcode::Bezr;
            break;
        case condition_t::lz:
            instr.opcode = Opcode::Blzr;
            break;
        case condition_t::gz:
            instr.opcode = Opcode::Bgzr;
            break;
        case condition_t::le:
            instr.opcode = Opcode::Bler;
            break;
        case condition_t::ge:
            instr.opcode = Opcode::Bger;
            break;
        }

        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }

    virtual void lil(reg_idx rD, s<18> imm) {
        instr.opcode = Opcode::Lil;
        instr.oper_regs[0] = rD;
        instr.imm = imm._sgn_inner();
    }
    virtual void lih(reg_idx rD, s<18> imm) {
        instr.opcode = Opcode::Lih;
        instr.oper_regs[0] = rD;
        instr.imm = imm._sgn_inner();
    }

    virtual void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) {
        if (b36)
            instr.opcode = Opcode::Ld36;
        else
            instr.opcode = Opcode::Ld32;

        instr.oper_regs[0] = rD;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }

    virtual void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) {
        if (b36)
            instr.opcode = Opcode::St36;
        else
            instr.opcode = Opcode::St32;

        instr.oper_regs[1] = rA;
        instr.oper_regs[2] = rB;
        instr.imm = imm._sgn_inner();
    }

    virtual void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                                  isa::ScalarArithmeticOp op) {
        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instr.opcode = Opcode::Add;
            break;
        case isa::ScalarArithmeticOp::Sub:
            instr.opcode = Opcode::Sub;
            break;
        case isa::ScalarArithmeticOp::Mul:
            instr.opcode = Opcode::Mul;
            break;
        case isa::ScalarArithmeticOp::And:
            instr.opcode = Opcode::And;
            break;
        case isa::ScalarArithmeticOp::Or:
            instr.opcode = Opcode::Or;
            break;
        case isa::ScalarArithmeticOp::Xor:
            instr.opcode = Opcode::Xor;
            break;
        case isa::ScalarArithmeticOp::Shr:
            instr.opcode = Opcode::Shr;
            break;
        case isa::ScalarArithmeticOp::Shl:
            instr.opcode = Opcode::Shl;
            break;
        }

        instr.oper_regs[0] = rD;
        instr.oper_regs[1] = rA;
        instr.oper_regs[2] = rB;
    }

    virtual void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                           isa::ScalarArithmeticOp op) {
        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instr.opcode = Opcode::Addi;
            break;
        case isa::ScalarArithmeticOp::Sub:
            instr.opcode = Opcode::Subi;
            break;
        case isa::ScalarArithmeticOp::And:
            instr.opcode = Opcode::Andi;
            break;
        case isa::ScalarArithmeticOp::Or:
            instr.opcode = Opcode::Ori;
            break;
        case isa::ScalarArithmeticOp::Xor:
            instr.opcode = Opcode::Xori;
            break;
        case isa::ScalarArithmeticOp::Shr:
            instr.opcode = Opcode::Shri;
            break;
        case isa::ScalarArithmeticOp::Shl:
            instr.opcode = Opcode::Shli;
            break;
        default:
            panic("invalid opcode for AI");
        }

        instr.oper_regs[0] = rD;
        instr.oper_regs[1] = rA;
        instr.imm = imm._sgn_inner();
    }
};

}; // namespace isa::disasm
