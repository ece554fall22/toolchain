#pragma once

#include <vector>

#include "ty.h"
#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mult, And, Or, Xor, Shr, Shl };
enum class FloatArithmeticOp { Fadd, Fsub, Fmult, Fdiv };
enum class VectorArithmeticOp {
    Vadd,
    Vsub,
    Vmult,
    Vdiv,
    Vdot,
    Vdota,
    Vindx,
    Vreduce,
    Vsplat,
    Vswizzle,
    Vsadd,
    Vsmult,
    Vssub,
    Vsdiv,
    Vsma,
    Vmax,
    Vmin,
    Vcompsel
};
enum class MatrixMultiplyOp {
    WriteA,
    WriteB,
    WriteC,
    Matmul,
    ReadC,
    Systolicstep
};
enum class CacheControlOp { Flushdirty, Flushclean, Flushicache, Flushline };
enum class CsrOp { Wcsr, Rcsr };
enum class FloatIntConversionOp { Ftoi, Itof };
enum class ConcurrencyOp { Fa, Cmpx };
enum class CmpMutateDirection { Increment, Decrement };
}; // namespace isa

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
    void vectorArithmetic(isa::VectorArithmeticOp op, vreg_idx vD, vreg_idx vA,
                          vreg_idx vB, reg_idx rD, reg_idx rA, reg_idx rB,
                          u<4> mask, s<8> imm);

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
    void csr(isa::CsrOp op, reg_idx rA, u<2> csrNum);
    void concurrency(isa::ConcurrencyOp op, reg_idx rD, reg_idx rA, reg_idx rB,
                     u<15> imm);

    auto getData() -> const auto& { return this->data; }

  private:
    void append(uint32_t enc) { data.push_back(enc); }

    std::vector<uint32_t> data;
};

}; // namespace isa
