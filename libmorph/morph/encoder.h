#pragma once

#include <vector>

#include "ty.h"
#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mult, And, Or, Xor, Shr, Shl };
enum class FloatArithmeticOp { Fadd, Fsub, Fmult, Fdiv };
enum class LanewiseVectorOp { Add, Sub, Mul, Div, Min, Max };
enum class VectorScalarOp {
    Add,
    Sub,
    Mul,
    Div,
};
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
    void jumpPCRel(s<25> imm, bool link);
    void jumpRegRel(reg_idx rA, s<20> imm, bool link);

    void branchImm(condition_t bt, s<22> imm);
    void branchReg(condition_t bt, reg_idx rA, s<17> imm);

    // scalar arithmetic
    void scalarArithmeticImmediate(isa::ScalarArithmeticOp op, reg_idx rD,
                                   reg_idx rA, s<15> imm);
    void scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD, reg_idx rA,
                          reg_idx rB);
    void scalarArithmeticNot(isa::ScalarArithmeticOp op, reg_idx rD,
                             reg_idx rA);

    void compareImm(reg_idx rA, s<20> imm);
    void compareReg(reg_idx rA, reg_idx rB);
    void compareAndMutate(isa::CmpMutateDirection dir, reg_idx rD, reg_idx rA);

    // scalar float arithmetic
    void floatArithmetic(isa::FloatArithmeticOp op, reg_idx rD, reg_idx rA,
                         reg_idx rB);

    // vector arithmetic
    void vecLanewiseArith(isa::LanewiseVectorOp op, vreg_idx vD, vreg_idx vA,
                          vreg_idx vB, vmask_t mask);
    void vectorScalarArith(isa::VectorScalarOp op, vreg_idx vD, reg_idx rA,
                           vreg_idx vB, vmask_t mask);
    void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB, vmask_t mask);
    void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB, vmask_t mask);
    void vidx(reg_idx rD, vreg_idx vA, vlaneidx_t idx);
    void vreduce(reg_idx rD, vreg_idx vA);
    void vsplat(vreg_idx vD, reg_idx rA);
    void vswizzle(vreg_idx vD, vreg_idx vA, vlaneidx_t idxs[4], vmask_t mask);
    void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, reg_idx vB, vmask_t mask);
    void vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vB, vmask_t mask);

    // matrix extensions
    void matrixMultiply(isa::MatrixMultiplyOp op, vreg_idx vD, vreg_idx vA,
                        vreg_idx vB, u<3> idx, bool high);

    // memory transfer
    void loadImmediate(bool high, reg_idx rD, u<18> imm);
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
    void flushCache(isa::CacheControlOp op, u<25> imm);
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
