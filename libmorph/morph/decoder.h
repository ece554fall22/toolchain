#pragma once

#include <cstdint>

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "isa.h"
#include "ty.h"
#include "util.h"
#include "varint.h"

namespace isa {
struct InstructionVisitor {
    virtual ~InstructionVisitor() = default;

    // misc
    virtual void nop() = 0;
    virtual void halt() = 0;
    virtual void bkpt(bits<25> imm) = 0;

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

    // A
    virtual void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                                  isa::ScalarArithmeticOp op) = 0;

    // AI
    virtual void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                           isa::ScalarArithmeticOp op) = 0;
};

void decodeInstruction(InstructionVisitor& visit, bits<32> instr);

// #define PRINT_RRR(fn, mnemonic)                                                \
//     virtual void fn(reg_idx rD, reg_idx rA, reg_idx rB) {                      \
//         fmt::print(#mnemonic " r{}, r{}, r{}", rD.inner, rA.inner, rB.inner);  \
//     }

struct PrintVisitor : public InstructionVisitor {
    explicit PrintVisitor(std::ostream& os) : os{os} {}
    virtual ~PrintVisitor() = default;

    // misc
    void nop() override { fmt::print(os, "nop"); }
    void halt() override { fmt::print(os, "halt"); }

    // J
    void jmp(s<25> imm) override { fmt::print(os, "jmp {:#x}", imm.inner); }
    void jal(s<25> imm) override { fmt::print(os, "jal {:#x}", imm.inner); }

    // JR
    void jmpr(reg_idx rA, s<20> imm) override {
        fmt::print(os, "jmpr r{}, {:#x}", rA.inner, imm.inner);
    }
    void jalr(reg_idx rA, s<20> imm) override {
        fmt::print(os, "jalr r{}, {:#x}", rA.inner, imm.inner);
    }

    // BI
    void branchimm(condition_t cond, s<22> imm) override {
        fmt::print(os, "b{}i {:#x}", cond, imm.inner);
    }
    // BR
    void branchreg(condition_t cond, reg_idx rA, s<17> imm) override {
        fmt::print(os, "b{}i r{}, {:#x}", cond, rA.inner, imm.inner);
    }

    void lil(reg_idx rD, s<18> imm) override {
        fmt::print(os, "lil r{}, {:#x}", rD, imm.inner);
    }
    void lih(reg_idx rD, s<18> imm) override {
        fmt::print(os, "lih r{}, {:#x}", rD, imm.inner);
    }

    void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) override {
        fmt::print(os, "ld{} r{}, [r{}+{:#x}]", b36 ? "36" : "32", rD.inner,
                   rA.inner, imm.inner);
    }

    void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) override {
        fmt::print(os, "st{} r{}, [r{}+{:#x}]", b36 ? "36" : "32", rA.inner,
                   rB.inner, imm.inner);
    }

    void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                          isa::ScalarArithmeticOp op) override {
        fmt::print(os, "{} r{}, r{}, r{}", op, rD.inner, rA.inner, rB.inner);
    }

    void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                   isa::ScalarArithmeticOp op) override {
        fmt::print(os, "{} r{}, r{}, {}", op, rD.inner, rA.inner,
                   imm._sgn_inner());
    }

    void bkpt(bits<25> imm) override {
        fmt::print(os, "bkpt {:#x}", imm.inner);
    }

  private:
    std::ostream& os;
};

// #undef PRINT_RRR

} // namespace isa
