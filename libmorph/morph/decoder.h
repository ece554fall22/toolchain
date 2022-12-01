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

    // CMPI
    virtual void cmpI(reg_idx rA, s<20> imm) = 0;

    // NOT
    virtual void arithmeticNot(reg_idx rD, reg_idx rA) = 0;

    // FA
    virtual void floatArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                          isa::FloatArithmeticOp op) = 0;

    // CMP
    virtual void cmp(reg_idx rA, reg_idx rB) = 0;

    // VA 
    virtual void vectorArithmetic(isa::VectorArithmeticOp op, vreg_idx vD, vreg_idx vA, 
                          vreg_idx vB, s<4> mask) = 0;

    // VDOT
    virtual void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) = 0;

    // VDOTA
    virtual void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) = 0;

    // VINDX
    virtual void vindx(reg_idx rD, vreg_idx vA, s<2> imm) = 0;

    // VREDUCE
    virtual void vreduce(reg_idx rD, vreg_idx vA, s<4> mask) = 0;

    // VSPLAT
    virtual void vsplat(vreg_idx vD, reg_idx rA, s<4> mask) = 0;

    // VSWIZZLE
    virtual void vswizzle(vreg_idx vD, vreg_idx vA, s<2>i1, s<2>i2, s<2> i3, s<2> i4, s<4> mask) = 0;

    // vector scalar ops
    virtual void vectorScalarArithmetic(isa::VectorScalarArithmeticOp op, vreg_idx vD, 
                                        reg_idx rA, vreg_idx vB, s<4> mask) = 0;
    
    // VSMA
    virtual void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB, s<4> mask) = 0;

    // Matrix write ops
    virtual void matrixWrite(isa::MatrixWriteOp op, s<3> idx, vreg_idx vA, vreg_idx vB) = 0;

    // Matmul
    virtual void matmul() = 0;

    // Systolicstep
    virtual void systolicstep() = 0;

    // ReadC
    virtual void readC(vreg_idx vD, s<3> idx, bool high) = 0;


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

    void cmpI(reg_idx rA, s<20> imm) override {
        fmt::print(os, "cmpi r{}, {:#x}", rA.inner, imm.inner);
    }

    void arithmeticNot(reg_idx rD, reg_idx rA) override {
        fmt::print(os, "not r{}, r{}", rD.inner, rA.inner);
    }

    void floatArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                          isa::FloatArithmeticOp op) override {
        fmt::print(os, "{} r{}, r{}, r{}", op, rD.inner, rA.inner, rB.inner);
    }

    void cmp(reg_idx rA, reg_idx rB) override {
        fmt::print(os, "cmp r{}, r{}", rA.inner, rB.inner);
    }

    void vectorArithmetic(isa::VectorArithmeticOp op, vreg_idx vD, vreg_idx vA, 
                          vreg_idx vB, s<4> mask) override {
        fmt::print(os, "{} v{}, v{}, v{}, {:#b}", op, vD.inner, vA.inner, vB.inner, mask.inner);
    }

    void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) override {
        fmt::print(os, "vdot r{}, v{}, v{}", rD.inner, vA.inner, vB.inner);
    }

    void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) override {
        fmt::print(os, "vdota r{}, r{}, v{}, v{}", rD.inner, rA.inner, vA.inner, vB.inner);
    }

    void vindx(reg_idx rD, vreg_idx vA, s<2> imm) override {
        fmt::print(os, "vindx r{}, v{}, {:#b}", rD.inner, vA.inner, imm.inner);
    }

    void vreduce(reg_idx rD, vreg_idx vA, s<4> mask) override {
        fmt::print(os, "vreduce r{}, v{}, {:#b}", rD.inner, vA.inner, mask.inner);
    }

    void vsplat(vreg_idx vD, reg_idx rA, s<4> mask) override {
        fmt::print(os, "vsplat v{}, r{}, {:#b}", vD.inner, rA.inner, mask.inner);
    }

    void vswizzle(vreg_idx vD, vreg_idx vA, s<2>i1, s<2>i2, s<2> i3, s<2> i4, s<4> mask) override {
        fmt::print(os, "vswizzle v{}, v{}, {:#b}, {:#b}, {:#b}, {:#b}, {:#b}", vD.inner, 
                    vA.inner, i1.inner, i2.inner, i3.inner, i4.inner, mask.inner);
    }

    void vectorScalarArithmetic(isa::VectorScalarArithmeticOp op, vreg_idx vD, 
                                        reg_idx rA, vreg_idx vB, s<4> mask) override {
        fmt::print(os, "{} v{}, r{}, v{} {:#b}", op, vD.inner, rA.inner, vB.inner, mask.inner);
    }

    void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB, s<4> mask) override {
        fmt::print(os, "vsma v{}, r{}, v{}, v{}, {:#b}", vD.inner, rA.inner, vA.inner, vB.inner, mask.inner);
    }

    void matrixWrite(isa::MatrixWriteOp op, s<3> idx, vreg_idx vA, vreg_idx vB) override {
        fmt::print(os, "{}, {:#x}, v{}, v{}", op, idx.inner, vA.inner, vB.inner);
    }

    void matmul() override { fmt::print(os, "matmul");}

    void systolicstep() override {fmt::print(os, "systolicstep");}

    void readC(vreg_idx vD, s<3> idx, bool high) override {
        fmt::print(os, "readC {}, {:#b}, v{}", high? 1 : 0, idx.inner, vD.inner);
    }

  private:
    std::ostream& os;
};

// #undef PRINT_RRR

} // namespace isa
