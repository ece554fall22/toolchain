#pragma once

#include <vector>

#include "ty.h"
#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mult, And, Or, Xor, Shr, Shl, Not };
enum class FloatArithmeticOp { Fadd, Fsub, Fmult, Fdiv};
enum class VectorArithmeticOp { Vadd, Vsub, Vmult, Vdiv, Vdot, Vdota, Vindx, Vreduce,
    Vsplat, Vswizzle, Vsadd, Vsmult, Vssub, Vsdiv, Vsma, Vmax, Vmin, Vcompsel};
enum class MatrixMultiplyOp { WriteA, WriteB, WriteC, Matmul, ReadC, Systolicstep};
enum class LoadStoreOp { Lih, Lil, Ld32, Ld36, St32, St36, Vldi, Vsti, Vldr, Vstr};
}; // namespace isa

class Emitter {
  public:
    Emitter() : data{} {}

    void jumpPCRel(s<25> imm, bool link);
    void jumpRegRel(reg_idx rA, s<20> imm, bool link);

    void scalarArithmeticImmediate(isa::ScalarArithmeticOp op, reg_idx rD,
                          reg_idx rA, s<15> imm);
    void scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD, reg_idx rA,
                          reg_idx rB);
    void floatArithmetic(isa::FloatArithmeticOp op, reg_idx rD, reg_idx rA,
                          reg_idx rB);
    void vectorArithmetic(isa::VectorArithmeticOp op, vreg_idx vD, vreg_idx vA,
                          vreg_idx vB, reg_idx rD, reg_idx rA, reg_idx rB, u<4> mask, s<8> imm);
    void matrixMultiply(isa::MatrixMultiplyOp op, vreg_idx vD, vreg_idx vA, vreg_idx vB,
                          u<3> idx, bool high);
    void loadStore(isa::LoadStoreOp op, vreg_idx vD, vreg_idx vA, reg_idx rD, reg_idx rA,
                        reg_idx rB, u<18> imm, u<4> mask);
                                 

    auto getData() -> const auto& { return this->data; }

  private:
    void append(uint32_t enc) { data.push_back(enc); }

    std::vector<uint32_t> data;
};

}; // namespace isa
