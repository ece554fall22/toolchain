#pragma once

#include <vector>

#include "isa.h"
#include "ty.h"
#include "util.h"
#include "varint.h"

namespace isa {
enum class MatrixMultiplyOp {
    WriteA,
    WriteB,
    WriteC,
    Matmul,
    ReadC,
    Systolicstep
};
enum class CacheControlOp { Flushdirty, Flushclean, Flushicache };
enum class CsrOp { Wcsr, Rcsr };
enum class FloatIntConversionOp { Ftoi, Itof };
enum class ConcurrencyOp { Fa, Cmpx };
enum class CmpMutateDirection { Increment, Decrement };

class Emitter {
  public:
    Emitter() : data{} {}

    // flow control
    void halt();
    void nop();
    void bkpt(bits<25> imm);

    void jumpPCRel(s<25> imm, bool link);
    void jumpRegRel(reg_idx rA, s<20> imm, bool link);

    void branchImm(condition_t bt, s<22> imm);
    void branchReg(condition_t bt, reg_idx rA, s<17> imm);

    // scalar arithmetic
    void scalarArithmeticImmediate(isa::ScalarArithmeticOp op, reg_idx rD,
                                   reg_idx rA, s<15> imm);
    void scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD, reg_idx rA,
                          reg_idx rB);
    void scalarArithmeticNot(reg_idx rD, reg_idx rA);

    void compareImm(reg_idx rA, s<20> imm);
    void compareReg(reg_idx rA, reg_idx rB);
    void compareAndMutate(isa::CmpMutateDirection dir, reg_idx rD, reg_idx rA);

    // scalar float arithmetic
    void floatArithmetic(isa::FloatArithmeticOp op, reg_idx rD, reg_idx rA,
                         reg_idx rB);

    // vector arithmetic
    void vectorLanewiseArith(isa::LanewiseVectorOp op, vreg_idx vD, vreg_idx vA,
                             vreg_idx vB, vmask_t mask);
    void vectorScalarArith(isa::VectorScalarOp op, vreg_idx vD, reg_idx rA,
                           vreg_idx vB, vmask_t mask);
    void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB, vmask_t mask);
    void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB, vmask_t mask);
    void vidx(reg_idx rD, vreg_idx vA, vlaneidx_t idx);
    void vreduce(reg_idx rD, vreg_idx vA);
    void vsplat(vreg_idx vD, reg_idx rA, vmask_t mask);
    void vswizzle(vreg_idx vD, vreg_idx vA, vlaneidx_t i0, vlaneidx_t i1, vlaneidx_t i2, vlaneidx_t i3, vmask_t mask);
    void vsma(vreg_idx vD, reg_idx rA, vreg_idx vB, vreg_idx vC, vmask_t mask);
    void vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vB, vmask_t mask);

    // matrix extensions
    void matrixMultiply(isa::MatrixMultiplyOp op, vreg_idx vD, vreg_idx vA,
                        vreg_idx vB, u<3> idx, bool high);

    // memory transfer
    void loadImmediate(bool high, reg_idx rD, bits<18> imm);
    void loadScalar(bool b36, reg_idx rD, reg_idx rA, s<15> imm);
    void storeScalar(bool b36, reg_idx rA, reg_idx rB, s<15> imm);

    void loadVectorImmStride(vreg_idx vD, reg_idx rA, s<11> imm, vmask_t mask);
    void storeVectorImmStride(reg_idx rA, vreg_idx vB, s<11> imm, vmask_t mask);
    void loadVectorRegStride(vreg_idx vD, reg_idx rA, reg_idx rB, vmask_t mask);
    void storeVectorRegStride(reg_idx rA, reg_idx rB, vreg_idx vA,
                              vmask_t mask);

    // scalar float stuff
    void floatIntConv(isa::FloatIntConversionOp op, reg_idx rD, reg_idx rA);

    // specials
    void flushcache(isa::CacheControlOp op);
    void flushline(u<25> imm);
    void csr(isa::CsrOp op, reg_idx rA, u<2> csrNum);
    void concurrency(isa::ConcurrencyOp op, reg_idx rD, reg_idx rA, reg_idx rB,
                     u<15> imm);

    auto getData() -> const auto& { return this->data; }

  private:
    void append(uint32_t enc) { data.push_back(enc); }

    std::vector<uint32_t> data;
};
}; // namespace isa
