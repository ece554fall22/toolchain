#include "morph/encoder.h"

using namespace isa;

#define BITFILL(n) ((1L << n) - 1)

uint32_t scalarArithmeticOpToAIOpcode(ScalarArithmeticOp op) {
    switch (op) {
    case ScalarArithmeticOp::Add:
        return 0b0010011;
    case ScalarArithmeticOp::Sub:
        return 0b0010100;
    case ScalarArithmeticOp::And:
        return 0b0010101;
    case ScalarArithmeticOp::Or:
        return 0b0010110;
    case ScalarArithmeticOp::Xor:
        return 0b0010111;
    case ScalarArithmeticOp::Shr:
        return 0b0011000;
    case ScalarArithmeticOp::Shl:
        return 0b0011001;
    default:
        panic("unsupported scalar arith op for AI format");
        return 0;
    }
}

uint32_t scalarArithmeticOpToArithCode(ScalarArithmeticOp op) {
    switch (op) {
    case ScalarArithmeticOp::Add:
        return 0b0000;
    case ScalarArithmeticOp::Sub:
        return 0b0001;
    case ScalarArithmeticOp::Mult:
        return 0b0010;
    case ScalarArithmeticOp::And:
        return 0b0011;
    case ScalarArithmeticOp::Or:
        return 0b0100;
    case ScalarArithmeticOp::Xor:
        return 0b0101;
    case ScalarArithmeticOp::Shr:
        return 0b0110;
    case ScalarArithmeticOp::Shl:
        return 0b0111;
    default:
        panic("unsupported scalar arith op for A format");
        return 0;
    }
}

uint32_t floatArithmeticOpToAOpcode(FloatArithmeticOp op) {
    switch (op) {
    case FloatArithmeticOp::Fadd:
    case FloatArithmeticOp::Fsub:
    case FloatArithmeticOp::Fmult:
    case FloatArithmeticOp::Fdiv:
        return 0b0011101;
    default:
        panic("unsupported float arith op");
        return 0;
    }
}

uint32_t floatArithmeticOpToArithCode(FloatArithmeticOp op) {
    switch (op) {
    case FloatArithmeticOp::Fadd:
        return 0b000;
    case FloatArithmeticOp::Fsub:
        return 0b001;
    case FloatArithmeticOp::Fmult:
        return 0b010;
    case FloatArithmeticOp::Fdiv:
        return 0b011;
    default:
        panic("unsupported float arith op");
        return 0;
    }
}

uint32_t vectorArithmeticOpToAOpcode(VectorArithmeticOp op) {
    switch (op) {
    case VectorArithmeticOp::Vadd:
        return 0b0011111;
    case VectorArithmeticOp::Vsub:
        return 0b0100000;
    case VectorArithmeticOp::Vmult:
        return 0b0100001;
    case VectorArithmeticOp::Vdiv:
        return 0b0100010;
    case VectorArithmeticOp::Vdot:
        return 0b0100011;
    case VectorArithmeticOp::Vdota:
        return 0b0100100;
    case VectorArithmeticOp::Vindx:
        return 0b0100101;
    case VectorArithmeticOp::Vreduce:
        return 0b0100110;
    case VectorArithmeticOp::Vsplat:
        return 0b0100111;
    case VectorArithmeticOp::Vswizzle:
        return 0b0101000;
    case VectorArithmeticOp::Vsadd:
        return 0b0101001;
    case VectorArithmeticOp::Vsmult:
        return 0b0101010;
    case VectorArithmeticOp::Vssub:
        return 0b0101011;
    case VectorArithmeticOp::Vsdiv:
        return 0b0101100;
    case VectorArithmeticOp::Vsma:
        return 0b0101101;
    case VectorArithmeticOp::Vmax:
        return 0b0110100;
    case VectorArithmeticOp::Vmin:
        return 0b0110101;
    case VectorArithmeticOp::Vcompsel:
        return 0b0110110;
    default:
        panic("unsupported vector arith op");
        return 0;
    }
}

uint32_t matrixMultiplyOpToAOpcode(MatrixMultiplyOp op) {
    switch (op) {
    case MatrixMultiplyOp::WriteA:
        return 0b0101110;
    case MatrixMultiplyOp::WriteB:
        return 0b0101111;
    case MatrixMultiplyOp::WriteC:
        return 0b0110000;
    case MatrixMultiplyOp::Matmul:
        return 0b0110001;
    case MatrixMultiplyOp::ReadC:
        return 0b0110010;
    case MatrixMultiplyOp::Systolicstep:
        return 0b0110011;
    default:
        panic("unsupported matrix multiply op");
        return 0;
    }
}

uint32_t cacheControlOpcode(CacheControlOp op) {
    switch (op) {
    case CacheControlOp::Flushdirty:
        return 0b0111101;
    case CacheControlOp::Flushclean:
        return 0b0111110;
    case CacheControlOp::Flushicache:
        return 0b0111111;
    case CacheControlOp::Flushline:
        return 0b1000000;
    default:
        panic("unsupported cache flush op");
        return 0;
    }
}

uint32_t branchCompareOpToAOpcode(BranchCompareOp op) {
    switch (op) {
    case BranchCompareOp::Bi:
        return 0b0000110;
    case BranchCompareOp::Br:
        return 0b0000111;
    case BranchCompareOp::Cmpi:
        return 0b0011010;
    case BranchCompareOp::Cmp:
        return 0b0011110;
    case BranchCompareOp::Cmpdec:
        return 0b1000001;
    case BranchCompareOp::Cmpinc:
        return 0b1000010;
    default:
        panic("unsupported branch/compare op");
        return 0;
    }
}

void Emitter::jumpPCRel(s<25> imm, bool link) {
    //                 opcode  | immediate offset
    //                link ---v
    uint32_t instr = 0b0000'010'0'0000'0000'0000'0000'0000'0000;
    if (link)
        instr |= (1 << 25);

    // immediate
    instr |= imm.inner; // TODO cleanup

    append(instr);
}

void isa::Emitter::jumpRegRel(reg_idx rA, s<20> imm, bool link) {
    //                 opcode  | imm | rA  | imm
    //                link ---v
    uint32_t instr = 0b0000'100'00000'00000'000'0000'0000'0000;
    if (link)
        instr |= (1 << 25);

    instr |= (rA.inner << 15);

    // immediate upper 5 bits.
    // TODO: better slicing
    auto immHigh = (imm.inner >> 15) & 0b11111;
    auto immLow = imm.inner & 0b111111111111111;

    instr |= immLow;
    instr |= (immHigh << 20);

    append(instr);
}

// ALLISON: scalar arith ops include add, sub, or, and, xor, shr, and shl
void Emitter::scalarArithmeticImmediate(isa::ScalarArithmeticOp op, reg_idx rD,
                                        reg_idx rA, s<15> imm) {
    uint32_t instr = 0;
    uint32_t opcode = scalarArithmeticOpToAIOpcode(op);

    // opcode
    instr |= (opcode << 25);

    // rD
    instr |= (rD.inner << 20);

    // rA
    instr |= (rA.inner << 15);

    // imm
    instr |= imm.inner & BITFILL(15);

    append(instr);
}

void Emitter::scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD,
                               reg_idx rA, reg_idx rB) {
    uint32_t instr = 0b0011011 << 25;

    // operand registers
    instr |= (rD.inner << 20);
    instr |= (rA.inner << 15);
    instr |= (rB.inner << 10);

    // arithmetic op
    instr |= scalarArithmeticOpToArithCode(op);

    append(instr);
}

void Emitter::scalarArithmeticNot(isa::ScalarArithmeticOp op, reg_idx rD,
                                  reg_idx rA) {
    uint32_t instr = 0b0011100 << 25;

    // operand registers
    instr |= (rD.inner << 20);
    instr |= (rA.inner << 15);

    append(instr);
}

void Emitter::floatArithmetic(isa::FloatArithmeticOp op, reg_idx rD, reg_idx rA,
                              reg_idx rB) {
    uint32_t instr = 0;

    uint32_t opcode = floatArithmeticOpToAOpcode(op);
    instr |= (opcode << 25);

    // rD
    instr |= (rD.inner << 20);

    // rA
    instr |= (rA.inner << 15);

    // rB
    instr |= (rB.inner << 10);

    // arithmetic op
    instr |= floatArithmeticOpToArithCode(op);

    append(instr);
}

// vector instructions: vadd, vsub, vmult, vdiv, vdot, vdota, vsadd, vsmult,
// vssub, vsdiv, vmax, vmin, vcompsel, vindx, vreduce, vsplat, vswizzle
void Emitter::vectorArithmetic(isa::VectorArithmeticOp op, vreg_idx vD,
                               vreg_idx vA, vreg_idx vB, reg_idx rD, reg_idx rA,
                               reg_idx rB, u<4> mask, s<8> imm) {
    uint32_t instr = 0;

    uint32_t opcode = vectorArithmeticOpToAOpcode(op);
    instr |= (opcode << 25);

    switch (op) {
    case VectorArithmeticOp::Vdot:
        instr |= (rD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (vB.inner << 10);
    case VectorArithmeticOp::Vdota:
        instr |= (rD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (vA.inner << 10);
        instr |= (vB.inner << 5);
    case VectorArithmeticOp::Vsma:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (vA.inner << 10);
        instr |= (vB.inner << 5);
        instr |= (mask.inner & 0b1111);
    case VectorArithmeticOp::Vindx:
        instr |= (rD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= ((imm.inner & 0b11111111)
                  << 7); // this immediate is 2 bits, but the extras
                         // will flow into don't care spots so i didn't
                         // adjust the size at all
    case VectorArithmeticOp::Vreduce:
        instr |= (rD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (mask.inner & 0b1111);
    case VectorArithmeticOp::Vsplat:
        instr |= (vD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (mask.inner & 0b1111);
    case VectorArithmeticOp::Vswizzle:
        instr |= (vD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= ((imm.inner & 0b11111111) << 7);
        instr |= (mask.inner & 0b1111);
    case VectorArithmeticOp::Vcompsel:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (rB.inner << 10);
        instr |= (vB.inner << 5);
        instr |= (mask.inner & 0b1111);
    // scalar+vector ops
    case VectorArithmeticOp::Vsadd:
    case VectorArithmeticOp::Vsmult:
    case VectorArithmeticOp::Vssub:
    case VectorArithmeticOp::Vsdiv:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (vA.inner << 10);
        instr |= (mask.inner & 0b1111);
    // vector ops
    case VectorArithmeticOp::Vadd:
    case VectorArithmeticOp::Vsub:
    case VectorArithmeticOp::Vmult:
    case VectorArithmeticOp::Vdiv:
    case VectorArithmeticOp::Vmax:
    case VectorArithmeticOp::Vmin:
        instr |= (vD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (vB.inner << 10);
        instr |= (mask.inner & 0b1111);
    default:
        panic("unsupported vector arith op");
        return;
    }

    append(instr);
}

// matmul, writea, writeb, writec, readc, systolicstep
void Emitter::matrixMultiply(isa::MatrixMultiplyOp op, vreg_idx vD, vreg_idx vA,
                             vreg_idx vB, u<3> idx, bool high) {

    uint32_t instr = 0;

    uint32_t opcode = matrixMultiplyOpToAOpcode(op);
    instr |= (opcode << 25);

    switch (op) {
    case MatrixMultiplyOp::Matmul:
    case MatrixMultiplyOp::Systolicstep:
        break;
    case MatrixMultiplyOp::ReadC:
        instr |= (vD.inner << 20);
        instr |= ((idx.inner & 0b111) << 17);
        if (high)
            instr |= (1 << 16);
    case MatrixMultiplyOp::WriteA:
    case MatrixMultiplyOp::WriteB:
    case MatrixMultiplyOp::WriteC:
        instr |= ((idx.inner & 0b111) << 20);
        instr |= (vA.inner << 15);
        instr |= (vB.inner << 10);
    default:
        panic("unsupported vector arith op");
        return;
    }

    append(instr);
}

// lih, lil
void Emitter::loadImmediate(bool hi, reg_idx rD, u<18> imm) {
    uint32_t instr = 0;
    if (hi)
        instr |= 0b0001000 << 25;
    else
        instr |= 0b0001001 << 25;

    instr |= rD.inner << 20;
    instr |= imm.inner;

    append(instr);
}

// ld32, ld36
void Emitter::loadScalar(bool b36, reg_idx rD, reg_idx rA, s<15> imm) {
    uint32_t instr = 0;
    // opcode
    if (b36)
        instr |= 0b0001011 << 25;
    else
        instr |= 0b0001010 << 25;

    // regs
    instr |= rD.inner << 20;
    instr |= rA.inner << 15;

    // immediate
    instr |= imm.inner;

    append(instr);
}

// st32, st36
void Emitter::storeScalar(bool b36, reg_idx rA, reg_idx rB, s<15> imm) {
    uint32_t instr = 0;
    // opcode
    if (b36)
        instr |= 0b0001101 << 25;
    else
        instr |= 0b0001100 << 25;
    ;

    // regs
    instr |= rA.inner << 15;
    instr |= rB.inner << 10;

    // immediate
    auto immHi = (imm.inner >> 10) & BITFILL(5);
    auto immLo = imm.inner & BITFILL(10);
    instr |= immHi << 20;
    instr |= immLo;

    append(instr);
}

void Emitter::loadVectorImmStride(vreg_idx vD, reg_idx rA, s<11> imm,
                                  vmask_t mask) {
    uint32_t instr = 0b0001110 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= imm.inner << 4;
    instr |= mask.inner;

    append(instr);
}

void Emitter::storeVectorImmStride(reg_idx rA, vreg_idx vB, s<11> imm,
                                   vmask_t mask) {
    uint32_t instr = 0b0010000 << 25;

    instr |= rA.inner << 15;
    instr |= vB.inner << 10;

    // immediate
    auto immHi = (imm.inner >> 6) & BITFILL(5);
    auto immLo = imm.inner & BITFILL(6);
    instr |= immHi << 20;
    instr |= immLo << 4;

    instr |= mask.inner;

    append(instr);
}

void Emitter::loadVectorRegStride(vreg_idx vD, reg_idx rA, reg_idx rB,
                                  vmask_t mask) {
    uint32_t instr = 0b0010001 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= rB.inner << 10;
    instr |= mask.inner;

    append(instr);
}

void Emitter::storeVectorRegStride(reg_idx rA, reg_idx rB, vreg_idx vA,
                                   vmask_t mask) {
    uint32_t instr = 0b0010010 << 25;

    instr |= rA.inner << 15;
    instr |= rB.inner << 10;
    instr |= vA.inner << 5;
    instr |= mask.inner;

    append(instr);
}

// flushdirty, flushclean, flushicache, flushline
void Emitter::flushCache(isa::CacheControlOp op, u<25> imm) {

    uint32_t instr = 0;
    uint32_t opcode = cacheControlOpcode(op);
    instr |= (opcode << 25);
    if (op == CacheControlOp::Flushline) {
        auto immHigh = (imm.inner >> 15) & BITFILL(10);
        auto immLow = imm.inner & BITFILL(15);
        instr |= immLow;
        instr |= (immHigh << 15);
    }

    append(instr);
}

// wcsr, rcsr
void Emitter::csr(isa::CsrOp op, u<2> csrNum) {

    uint32_t instr = 0;

    if (op == CsrOp::Wcsr) {
        instr |= (0b0111001 << 25);
    } else if (op == CsrOp::Rcsr) {
        instr |= (0b0111000 << 25);
    } else {
        panic("unsupported csr op");
        return;
    }
    instr |= (csrNum.inner & 0b11) << 23;

    append(instr);
}

// ftoi, itof
void Emitter::floatIntConv(isa::FloatIntConversionOp op, reg_idx rD,
                           reg_idx rA) {

    uint32_t instr = 0;

    if (op == FloatIntConversionOp::Ftoi) {
        instr |= (0b0110111 << 25);
    } else if (op == FloatIntConversionOp::Itof) {
        instr |= (0b0111000);
    } else {
        panic("unsupported float int conversion op");
        return;
    }

    instr |= (rD.inner << 20);
    instr |= (rA.inner << 15);

    append(instr);
}

// fa, cmpx
void Emitter::concurrency(isa::ConcurrencyOp op, reg_idx rD, reg_idx rA,
                          reg_idx rB, u<15> imm) {

    uint32_t instr = 0;

    if (op == ConcurrencyOp::Fa) {
        instr |= (0b0111011 << 25);
        instr |= (rD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= imm.inner & BITFILL(15);
    } else if (op == ConcurrencyOp::Cmpx) {
        instr |= (0b0111100 << 25);
        instr |= (rD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (rB.inner << 10);
    } else {
        panic("unsupported concurrency op");
        return;
    }

    append(instr);
}

// bi, br, cmpi, cmp, cmpdec, cmpinc
void Emitter::branchCompare(isa::BranchCompareOp op, reg_idx rD, reg_idx rA,
                            reg_idx rB, u<22> imm, u<3> btx) {

    uint32_t instr = 0;
    uint32_t opcode = branchCompareOpToAOpcode(op);
    instr |= (opcode << 25);

    switch (op) {
    case BranchCompareOp::Bi:
        instr |= ((btx.inner & 0b111) << 22);
        instr |= imm.inner & BITFILL(22);
    case BranchCompareOp::Br:
        instr |= ((btx.inner & 0b111) << 22);
        instr |= ((imm.inner >> 15) & 0b11) << 20;
        instr |= rA.inner << 15;
        instr |= imm.inner & BITFILL(15);
    case BranchCompareOp::Cmpi:
        instr |= ((imm.inner >> 15) & 0b11111) << 20;
        instr |= rA.inner << 15;
        instr |= imm.inner & BITFILL(15);
    case BranchCompareOp::Cmp:
        instr |= rA.inner << 15;
        instr |= rB.inner << 10;
    case BranchCompareOp::Cmpdec:
    case BranchCompareOp::Cmpinc:
        instr |= rD.inner << 20;
        instr |= rA.inner << 15;
        instr |= rB.inner << 10;
    default:
        panic("unsupported branch/compare op");
        return;
    }

    append(instr);
}

// misc
void Emitter::halt() {
    uint32_t instr = 0b0000000 << 25;

    append(instr);
}

void Emitter::nop() {
    uint32_t instr = 0b0000001 << 25;

    append(instr);
}
