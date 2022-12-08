#include "morph/encoder.h"

#include "morph/ty.h"

using namespace isa;

#define BITFILL(n) ((1L << n) - 1)

// defensive against any changes in ty.h; we need these assumptions for
// following bitmanip code in the instruction emitter
static_assert(reg_idx::size == 5, "scalar register indices should be 5 bits");
static_assert(vreg_idx::size == 5, "vector register indices should be 5 bits");
static_assert(vmask_t::size == 4, "vector masks should be 4 bits");
static_assert(vlaneidx_t::size == 2, "vector lane indices should be 2 bits");

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
    case ScalarArithmeticOp::Mul:
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

uint32_t floatArithmeticOpToArithCode(FloatArithmeticOp op) {
    switch (op) {
    case FloatArithmeticOp::Fadd:
        return 0b000;
    case FloatArithmeticOp::Fsub:
        return 0b001;
    case FloatArithmeticOp::Fmul:
        return 0b010;
    case FloatArithmeticOp::Fdiv:
        return 0b011;
    default:
        panic("unsupported float arith op");
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
    default:
        panic("unsupported cache flush op");
        return 0;
    }
}

void isa::Emitter::jumpPCRel(s<25> imm, bool link) {
    //                 opcode  | immediate offset
    //                link ---v
    uint32_t instr = 0b0000'010'0'0000'0000'0000'0000'0000'0000;
    if (link)
        instr |= (1 << 25);

    // immediate
    instr |= imm.raw(); // TODO cleanup

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
    auto immHigh = (imm.raw() >> 15) & BITFILL(5);
    auto immLow = imm.raw() & BITFILL(15);

    instr |= immLow;
    instr |= (immHigh << 20);

    append(instr);
}

void isa::Emitter::branchImm(condition_t bt, s<22> imm) {
    uint32_t instr = 0b0000110 << 25;

    instr |= static_cast<uint32_t>(bt) << 22;
    instr |= imm.raw();

    append(instr);
}

void isa::Emitter::branchReg(condition_t bt, reg_idx rA, s<17> imm) {
    uint32_t instr = 0b0000111 << 25;

    instr |= static_cast<uint32_t>(bt) << 22;
    instr |= rA.inner << 15;
    instr |= imm.raw();

    append(instr);
}

// ALLISON: scalar arith ops include add, sub, or, and, xor, shr, and shl
void isa::Emitter::scalarArithmeticImmediate(isa::ScalarArithmeticOp op,
                                             reg_idx rD, reg_idx rA,
                                             s<15> imm) {
    uint32_t instr = 0;
    uint32_t opcode = scalarArithmeticOpToAIOpcode(op);

    // opcode
    instr |= (opcode << 25);

    // rD
    instr |= (rD.inner << 20);

    // rA
    instr |= (rA.inner << 15);

    // imm
    instr |= imm.raw() & BITFILL(15);

    append(instr);
}

void isa::Emitter::scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD,
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

void isa::Emitter::scalarArithmeticNot(reg_idx rD, reg_idx rA) {
    uint32_t instr = 0b0011100 << 25;

    // operand registers
    instr |= (rD.inner << 20);
    instr |= (rA.inner << 15);

    append(instr);
}

// comparisons
void isa::Emitter::compareImm(reg_idx rA, s<20> imm) {
    uint32_t instr = 0b0011010 << 25;

    auto immHi = (imm.raw() >> 15) & BITFILL(5);
    auto immLo = imm.raw() & BITFILL(15);
    instr |= immHi << 20;
    instr |= rA.inner << 15;
    instr |= immLo;

    append(instr);
}

void isa::Emitter::compareReg(reg_idx rA, reg_idx rB) {
    uint32_t instr = 0b0011110 << 25;

    instr |= rA.inner << 15;
    instr |= rB.inner << 10;

    append(instr);
}

void isa::Emitter::compareAndMutate(CmpMutateDirection dir, reg_idx rD,
                                    reg_idx rA, reg_idx rB) {
    uint32_t instr = 0b1000000 << 25;
    switch (dir) {
    case CmpMutateDirection::Increment:
        instr |= 0b01 << 25;
        break;
    case CmpMutateDirection::Decrement:
        instr |= 0b10 << 25;
        break;
    }

    instr |= rD.raw() << 20;
    instr |= rA.raw() << 15;
    instr |= rB.raw() << 10;

    append(instr);
}

void isa::Emitter::floatArithmetic(isa::FloatArithmeticOp op, reg_idx rD,
                                   reg_idx rA, reg_idx rB) {
    uint32_t instr = 0b0011101 << 25;

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

// vadd, vsub, vmul, vdiv, vmax, vmin
void isa::Emitter::vectorLanewiseArith(isa::LanewiseVectorOp op, vreg_idx vD,
                                       vreg_idx vA, vreg_idx vB, vmask_t mask) {
    uint32_t instr = 0;
    switch (op) {
    case LanewiseVectorOp::Add:
        instr = 0b0011111 << 25;
        break;
    case LanewiseVectorOp::Sub:
        instr = 0b0100000 << 25;
        break;
    case LanewiseVectorOp::Mul:
        instr = 0b0100001 << 25;
        break;
    case LanewiseVectorOp::Div:
        instr = 0b0100010 << 25;
        break;
    case LanewiseVectorOp::Max:
        instr = 0b0110100 << 25;
        break;
    case LanewiseVectorOp::Min:
        instr = 0b0110101 << 25;
        break;
    }

    instr |= vD.inner << 20;
    instr |= vA.inner << 15;
    instr |= vB.inner << 10;
    instr |= mask.inner;

    append(instr);
}

// vsadd, vsmul, vssub, vsdiv
void isa::Emitter::vectorScalarArith(isa::VectorScalarOp op, vreg_idx vD,
                                     reg_idx rA, vreg_idx vB, vmask_t mask) {
    uint32_t instr = 0;
    switch (op) {
    case VectorScalarOp::Add:
        instr = 0b0101001 << 25;
        break;
    case VectorScalarOp::Mul:
        instr = 0b0101010 << 25;
        break;
    case VectorScalarOp::Sub:
        instr = 0b0101011 << 25;
        break;
    case VectorScalarOp::Div:
        instr = 0b0101100 << 25;
        break;
    }

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= vB.inner << 10;
    instr |= mask.inner;

    append(instr);
}

void isa::Emitter::vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) {
    uint32_t instr = 0b0100011 << 25;

    instr |= rD.inner << 20;
    instr |= vA.inner << 15;
    instr |= vB.inner << 10;

    append(instr);
}

void isa::Emitter::vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) {
    uint32_t instr = 0b0100100 << 25;

    instr |= rD.inner << 20;
    instr |= rA.inner << 15;
    instr |= vA.inner << 10;
    instr |= vB.inner << 5;

    append(instr);
}

void isa::Emitter::vidx(reg_idx rD, vreg_idx vA, vlaneidx_t idx) {
    uint32_t instr = 0b0100101 << 25;

    instr |= rD.inner << 20;
    instr |= vA.inner << 15;
    instr |= idx.inner << 7;

    append(instr);
}

void isa::Emitter::vreduce(reg_idx rD, vreg_idx vA, vmask_t mask) {
    uint32_t instr = 0b0100110 << 25;

    instr |= rD.inner << 20;
    instr |= vA.inner << 15;
    instr |= mask.raw();

    append(instr);
}

void isa::Emitter::vsplat(vreg_idx vD, reg_idx rA, vmask_t mask) {
    uint32_t instr = 0b0100111 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= mask.raw();

    append(instr);
}

void isa::Emitter::vswizzle(vreg_idx vD, vreg_idx vA, vlaneidx_t i0,
                            vlaneidx_t i1, vlaneidx_t i2, vlaneidx_t i3,
                            vmask_t mask) {
    uint32_t instr = 0b0101000 << 25;

    instr |= vD.inner << 20;
    instr |= vA.inner << 15;
    instr |= i3.inner << 13;
    instr |= i2.inner << 11;
    instr |= i1.inner << 9;
    instr |= i0.inner << 7;
    instr |= mask.inner;

    append(instr);
}

void isa::Emitter::vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB,
                        vmask_t mask) {
    uint32_t instr = 0b0101101 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= vA.inner << 10;
    instr |= vB.inner << 5;
    instr |= mask.inner;

    append(instr);
}

void isa::Emitter::vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vB,
                         vmask_t mask) {
    uint32_t instr = 0b0110110 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= rB.inner << 10;
    instr |= vB.inner << 5;
    instr |= mask.inner;

    append(instr);
}

void Emitter::matrixWrite(isa::MatrixWriteOp op, vreg_idx vA, vreg_idx vB,
                          u<3> row) {
    uint32_t instr = 0;

    switch (op) {
    case MatrixWriteOp::WriteA:
        instr |= 0b0101110 << 25;
        break;
    case MatrixWriteOp::WriteB:
        instr |= 0b0101111 << 25;
        break;
    case MatrixWriteOp::WriteC:
        instr |= 0b0110000 << 25;
        break;
    }

    instr |= row.raw() << 20;
    instr |= vA.raw() << 15;
    instr |= vB.raw() << 10;

    append(instr);
}

void isa::Emitter::systolicStep() { append(0b0110011 << 25); }

void isa::Emitter::matmul() { append(0b0110001 << 25); }

void isa::Emitter::readC(vreg_idx vD, u<3> row, bool high) {
    uint32_t instr = 0b0110010 << 25;

    instr |= (vD.inner << 20);
    instr |= (row.raw() << 17);
    if (high)
        instr |= 1 << 16;

    append(instr);
}

// lih, lil
void isa::Emitter::loadImmediate(bool hi, reg_idx rD, bits<18> imm) {
    uint32_t instr = 0;
    if (hi)
        instr |= 0b0001000 << 25;
    else
        instr |= 0b0001001 << 25;

    instr |= rD.inner << 20;
    instr |= imm.raw();

    append(instr);
}

// ld32, ld36
void isa::Emitter::loadScalar(bool b36, reg_idx rD, reg_idx rA, s<15> imm) {
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
    instr |= imm.raw();

    append(instr);
}

// st32, st36
void isa::Emitter::storeScalar(bool b36, reg_idx rA, reg_idx rB, s<15> imm) {
    uint32_t instr = 0;
    // opcode
    if (b36)
        instr |= 0b0001101 << 25;
    else
        instr |= 0b0001100 << 25;

    // regs
    instr |= rA.inner << 15;
    instr |= rB.inner << 10;

    // immediate
    auto immHi = (imm.raw() >> 10) & BITFILL(5);
    auto immLo = imm.raw() & BITFILL(10);
    instr |= immHi << 20;
    instr |= immLo;

    append(instr);
}

void isa::Emitter::loadVectorImmStride(vreg_idx vD, reg_idx rA, s<11> imm,
                                       vmask_t mask) {
    uint32_t instr = 0b0001110 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= imm.raw() << 4;
    instr |= mask.inner;

    append(instr);
}

void isa::Emitter::storeVectorImmStride(reg_idx rA, vreg_idx vB, s<11> imm,
                                        vmask_t mask) {
    uint32_t instr = 0b0010000 << 25;

    instr |= rA.inner << 15;
    instr |= vB.inner << 10;

    // immediate
    auto immHi = (imm.raw() >> 6) & BITFILL(5);
    auto immLo = imm.raw() & BITFILL(6);
    instr |= immHi << 20;
    instr |= immLo << 4;

    instr |= mask.inner;

    append(instr);
}

void isa::Emitter::loadVectorRegStride(vreg_idx vD, reg_idx rA, reg_idx rB,
                                       vmask_t mask) {
    uint32_t instr = 0b0010001 << 25;

    instr |= vD.inner << 20;
    instr |= rA.inner << 15;
    instr |= rB.inner << 10;
    instr |= mask.inner;

    append(instr);
}

void isa::Emitter::storeVectorRegStride(reg_idx rA, reg_idx rB, vreg_idx vA,
                                        vmask_t mask) {
    uint32_t instr = 0b0010010 << 25;

    instr |= rA.inner << 15;
    instr |= rB.inner << 10;
    instr |= vA.inner << 5;
    instr |= mask.inner;

    append(instr);
}

// flushdirty, flushclean, flushicache, flushline
// TODO: split up
void isa::Emitter::flushcache(isa::CacheControlOp op) {
    uint32_t instr = cacheControlOpcode(op) << 25;
    append(instr);
}

void isa::Emitter::flushline(reg_idx rA, s<20> imm) {
    uint32_t instr = 0b1000000 << 25;

    auto immHigh = (imm.raw() >> 15) & BITFILL(5);
    auto immLow = imm.raw() & BITFILL(15);
    instr |= immLow;
    instr |= rA.raw() << 15;
    instr |= (immHigh << 20);

    append(instr);
}

// wcsr, rcsr
void isa::Emitter::csr(isa::CsrOp op, reg_idx rA, u<2> csrNum) {
    uint32_t instr = 0;
    switch (op) {
    case CsrOp::Wcsr:
        instr |= (0b0111001 << 25);
        break;
    case CsrOp::Rcsr:
        instr |= (0b0111000 << 25);
        break;
    }
    instr |= csrNum.inner << 23;
    instr |= rA.inner << 15;

    append(instr);
}

// ftoi, itof
void isa::Emitter::floatIntConv(isa::FloatIntConversionOp op, reg_idx rD,
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
void isa::Emitter::concurrency(isa::ConcurrencyOp op, reg_idx rD, reg_idx rA,
                               reg_idx rB, u<15> imm) {

    uint32_t instr = 0;

    if (op == ConcurrencyOp::Fa) {
        instr |= (0b0111011 << 25);
        instr |= (rD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= imm.raw() & BITFILL(15);
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

// misc
void isa::Emitter::halt() {
    uint32_t instr = 0b0000000 << 25;

    append(instr);
}

void isa::Emitter::nop() {
    uint32_t instr = 0b0000001 << 25;

    append(instr);
}

void isa::Emitter::bkpt(bits<25> imm) {
    uint32_t instr = 0b1010101 << 25;
    instr |= imm.raw();

    append(instr);
}
