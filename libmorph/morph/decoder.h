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

    // Vldi
    virtual void vldi(vreg_idx vD, reg_idx rA, s<11> imm, vmask_t mask) = 0;

    // Vsti
    virtual void vsti(s<11> imm, reg_idx rA, vreg_idx vB, vmask_t mask) = 0;

    // Vldr
    virtual void vldr(vreg_idx vD, reg_idx rA, reg_idx rB, vmask_t mask) = 0;

    // Vstr
    virtual void vstr(reg_idx rA, reg_idx rB, vreg_idx vA, vmask_t mask) = 0;

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
    virtual void vectorArithmetic(isa::LanewiseVectorOp op, vreg_idx vD,
                                  vreg_idx vA, vreg_idx vB, vmask_t mask) = 0;

    // VDOT
    virtual void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) = 0;

    // VDOTA
    virtual void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) = 0;

    // VIDX
    virtual void vidx(reg_idx rD, vreg_idx vA, vlaneidx_t imm) = 0;

    // VREDUCE
    virtual void vreduce(reg_idx rD, vreg_idx vA, vmask_t mask) = 0;

    // VSPLAT
    virtual void vsplat(vreg_idx vD, reg_idx rA, vmask_t mask) = 0;

    // VSWIZZLE
    virtual void vswizzle(vreg_idx vD, vreg_idx vA, vlaneidx_t i0,
                          vlaneidx_t i1, vlaneidx_t i2, vlaneidx_t i3,
                          vmask_t mask) = 0;

    // vector scalar ops
    virtual void vectorScalarArithmetic(isa::VectorScalarOp op, vreg_idx vD,
                                        reg_idx rA, vreg_idx vB,
                                        vmask_t mask) = 0;

    // VSMA
    virtual void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB,
                      vmask_t mask) = 0;

    // Matrix write ops
    virtual void matrixWrite(isa::MatrixWriteOp op, s<3> idx, vreg_idx vA,
                             vreg_idx vB) = 0;

    // Matmul
    virtual void matmul() = 0;

    // Systolicstep
    virtual void systolicstep() = 0;

    // ReadC
    virtual void readC(vreg_idx vD, s<3> idx, bool high) = 0;

    // Vcomp
    virtual void vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vB,
                       vmask_t mask) = 0;

    // Flushdirty
    virtual void flushdirty() = 0;

    // Flushclean
    virtual void flushclean() = 0;

    // Flushicache
    virtual void flushicache() = 0;

    // Flushline
    virtual void flushline(reg_idx rA, s<20> imm) = 0;

    // Fa
    virtual void fa(reg_idx rD, reg_idx rA, s<15> imm) = 0;

    // Cmpx
    virtual void cmpx(reg_idx rD, reg_idx rA, s<15> imm) = 0;

    // FtoI
    virtual void ftoi(reg_idx rD, reg_idx rA) = 0;

    // Itof
    virtual void itof(reg_idx rD, reg_idx rA) = 0;

    // Wcsr
    virtual void wcsr(s<2> csr, reg_idx rA) = 0;

    // Rcsr
    virtual void rcsr(s<2> csr, reg_idx rA) = 0;

    // Cmpdec
    virtual void cmpdec(reg_idx rD, reg_idx rA, reg_idx rB) = 0;

    // Cmpinc
    virtual void cmpinc(reg_idx rD, reg_idx rA, reg_idx rB) = 0;
};

void decodeInstruction(InstructionVisitor& visit, bits<32> instr);

struct PrintVisitor : public InstructionVisitor {
    explicit PrintVisitor(std::ostream& os) : os{os} {}
    virtual ~PrintVisitor() = default;

    // misc
    void nop() override { fmt::print(os, "nop"); }
    void halt() override { fmt::print(os, "halt"); }

    // J
    void jmp(s<25> imm) override { fmt::print(os, "jmp {:#x}", imm._sgn_inner()); }
    void jal(s<25> imm) override { fmt::print(os, "jal {:#x}", imm._sgn_inner()); }

    // JR
    void jmpr(reg_idx rA, s<20> imm) override {
        fmt::print(os, "jmpr r{}, {:#x}", rA.inner, imm._sgn_inner());
    }
    void jalr(reg_idx rA, s<20> imm) override {
        fmt::print(os, "jalr r{}, {:#x}", rA.inner, imm._sgn_inner());
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
        fmt::print(os, "lil r{}, {:#x}", rD, imm.raw());
    }
    void lih(reg_idx rD, s<18> imm) override {
        fmt::print(os, "lih r{}, {:#x}", rD, imm.raw());
    }

    void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) override {
        fmt::print(os, "ld{} r{}, [r{}+{:#x}]", b36 ? "36" : "32", rD.inner,
                   rA.inner, imm.inner);
    }

    void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) override {
        fmt::print(os, "st{} [r{}+{:#x}], r{}", b36 ? "36" : "32", rB.inner,
                   imm.inner, rA.inner);
    }

    void vldi(vreg_idx vD, reg_idx rA, s<11> imm, vmask_t mask) override {
        fmt::print(os, "vldi {:#b}, v{}, [r{}+={:#x}]", mask.inner, vD.inner,
                   rA.inner, imm.inner);
    }

    void vsti(s<11> imm, reg_idx rA, vreg_idx vB, vmask_t mask) override {
        fmt::print(os, "vldi {:#b}, [r{}+={:#x}], v{}", mask.inner, rA.inner,
                   imm.inner, vB.inner);
    }

    void vldr(vreg_idx vD, reg_idx rA, reg_idx rB, vmask_t mask) override {
        fmt::print(os, "vldr {:#b}, v{}, [r{}+=r{}]", mask.inner, vD.inner,
                   rA.inner, rB.inner);
    }

    void vstr(reg_idx rA, reg_idx rB, vreg_idx vA, vmask_t mask) override {
        fmt::print(os, "vldr {:#b}, v{}, [r{}+=r{}]", mask.inner, rA.inner,
                   rB.inner, vA.inner);
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

    void vectorArithmetic(isa::LanewiseVectorOp op, vreg_idx vD, vreg_idx vA,
                          vreg_idx vB, vmask_t mask) override {
        fmt::print(os, "v{} v{}, v{}, v{}, {:#b}", op, vD.inner, vA.inner,
                   vB.inner, mask.inner);
    }

    void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) override {
        fmt::print(os, "vdot r{}, v{}, v{}", rD.inner, vA.inner, vB.inner);
    }

    void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) override {
        fmt::print(os, "vdota r{}, r{}, v{}, v{}", rD.inner, rA.inner, vA.inner,
                   vB.inner);
    }

    void vidx(reg_idx rD, vreg_idx vA, u<2> imm) override {
        fmt::print(os, "vidx r{}, v{}, {:#b}", rD.inner, vA.inner, imm.inner);
    }

    void vreduce(reg_idx rD, vreg_idx vA, vmask_t mask) override {
        fmt::print(os, "vreduce r{}, v{}, {:#b}", rD.inner, vA.inner,
                   mask.inner);
    }

    void vsplat(vreg_idx vD, reg_idx rA, vmask_t mask) override {
        fmt::print(os, "vsplat v{}, r{}, {:#b}", vD.inner, rA.inner,
                   mask.inner);
    }

    void vswizzle(vreg_idx vD, vreg_idx vA, vlaneidx_t i0, vlaneidx_t i1,
                  vlaneidx_t i2, vlaneidx_t i3, vmask_t mask) override {
        fmt::print(os, "vswizzle {:#b}, v{}, v{}, {:#b}, {:#b}, {:#b}, {:#b}",
                   mask.inner, vD.inner, vA.inner, i0.inner, i1.inner, i2.inner,
                   i3.inner);
    }

    void vectorScalarArithmetic(isa::VectorScalarOp op, vreg_idx vD, reg_idx rA,
                                vreg_idx vB, vmask_t mask) override {
        fmt::print(os, "vs{} v{}, r{}, v{} {:#b}", op, vD.inner, rA.inner,
                   vB.inner, mask.inner);
    }

    void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB,
              vmask_t mask) override {
        fmt::print(os, "vsma v{}, r{}, v{}, v{}, {:#b}", vD.inner, rA.inner,
                   vA.inner, vB.inner, mask.inner);
    }

    void matrixWrite(isa::MatrixWriteOp op, s<3> idx, vreg_idx vA,
                     vreg_idx vB) override {
        fmt::print(os, "{}, {:#x}, v{}, v{}", op, idx.inner, vA.inner,
                   vB.inner);
    }

    void matmul() override { fmt::print(os, "matmul"); }

    void systolicstep() override { fmt::print(os, "systolicstep"); }

    void readC(vreg_idx vD, s<3> idx, bool high) override {
        fmt::print(os, "readC {}, {:#b}, v{}", high ? 1 : 0, idx.inner,
                   vD.inner);
    }

    void vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vB,
               vmask_t mask) override {
        fmt::print(os, "vcomp v{}, r{}, r{}, v{}, {:#b}", vD.inner, rA.inner,
                   rB.inner, vB.inner, mask.inner);
    }

    void flushdirty() override { fmt::print(os, "flushdirty"); }

    void flushclean() override { fmt::print(os, "flushclean"); }

    void flushicache() override { fmt::print(os, "flushicache"); }

    void flushline(reg_idx rA, s<20> imm) override {
        fmt::print(os, "flushline r{}, {:#x}", rA.inner, imm._sgn_inner());
    }

    void fa(reg_idx rD, reg_idx rA, s<15> imm) override {
        fmt::print(os, "fa r{}, r{}, {:#x}", rD.inner, rA.inner, imm.inner);
    }

    void cmpx(reg_idx rD, reg_idx rA, s<15> imm) override {
        fmt::print(os, "cmpx r{}, r{}, {:#x}", rD.inner, rA.inner, imm.inner);
    }

    void ftoi(reg_idx rD, reg_idx rA) override {
        fmt::print(os, "ftoi r{}, r{}", rD.inner, rA.inner);
    }

    void itof(reg_idx rD, reg_idx rA) override {
        fmt::print(os, "itof r{}, r{}", rD.inner, rA.inner);
    }

    void wcsr(s<2> csr, reg_idx rA) override {
        fmt::print(os, "wcsr c{}, r{}", csr.inner, rA.inner);
    }

    void rcsr(s<2> csr, reg_idx rA) override {
        fmt::print(os, "rcsr c{}, r{}", csr.inner, rA.inner);
    }

    void cmpdec(reg_idx rD, reg_idx rA, reg_idx rB) override {
        fmt::print(os, "cmpdec r{}, r{}, r{}", rD.inner, rA.inner, rB.inner);
    }

    void cmpinc(reg_idx rD, reg_idx rA, reg_idx rB) override {
        fmt::print(os, "cmpinc r{}, r{}, r{}", rD.inner, rA.inner, rB.inner);
    }

  private:
    std::ostream& os;
};

// #undef PRINT_RRR

} // namespace isa
