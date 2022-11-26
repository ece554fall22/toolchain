#include "morph/encoder.h"

using isa::ScalarArithmeticOp;
using isa::FloatArithmeticOp;


// TODO: halt, nop, bi, br, lih, lil, ld32, ld36, st32, st36, vldi, vsti, vldr, vstr, not, cmp,
// writea, writeb, writec,
// matmul, readc, systolicstep, ftoi, itof, wcsr, rcsr, fa, cmpx, flushdirty, flushclean, flushicache,
// flushline, cmpdec, cmpinc

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

uint32_t scalarArithmeticOpToAOpcode(ScalarArithmeticOp op) {
    switch (op) {
    case ScalarArithmeticOp::Add:
    case ScalarArithmeticOp::Sub:
    case ScalarArithmeticOp::Mult:
    case ScalarArithmeticOp::And:
    case ScalarArithmeticOp::Or:
    case ScalarArithmeticOp::Xor:
    case ScalarArithmeticOp::Shr:
    case ScalarArithmeticOp::Shl:
        return 0b0011011;
    default:
        panic("unsupported scalar arith op for A format");
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
    switch(op) {
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
    case vectorArithmeticOp::Vindx:
        return 0b0100101;
    case vectorArithmeticOp::Vreduce:
        return 0b0100110;
    case vectorArithmeticOp::Vsplat:
        return 0b0100111;
    case vectorArithmeticOp::Vswizzle:
        return 0b0101000;
    case vectorArithmeticOp::Vsadd:
        return 0b0101001;
    case vectorArithmeticOp::Vsmult:
        return 0b0101010;
    case vectorArithmeticOp::Vssub:
        return 0b0101011;
    case vectorArithmeticOp::Vsdiv:
        return 0b0101100;
    case vectorArithmeticOp::Vsma:
        return 0b0101101;
    case vectorArithmeticOp::Vmax:
        return 0b0110100;
    case vectorArithmeticOp::Vmin:
        return 0b0110101;
    case vectorArithmeticOp::Vcompsel:
        return 0b0110110;
    default:
        panic("unsupported vector arith op");
        return 0;
    }
}

uint32_t matrixMultipyOpToAOpcode(VectorArithmeticOp op) {
    switch(op) {
    case matrixMultiplyOp::writeA:
        return 0b0101110;
    case matrixMultipyOp::writeB:
        return 0b0101111;
    case matrixMultiplyOp::writeC:
        return 0b0110000;
    case matrixMultipyOp::matmul:
        return 0b0110001;
    case matrixMultipyOp::readC:
        return 0b0110010;
    case matrixMultipyOp::systolicstep:
        return 0b0110011;
    default:
        panic("unsupported matrix multiply op");
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

void Emitter::jumpRegRel(reg_idx rA, s<20> imm, bool link) {
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
    instr |= (imm.inner & 0b111111111111111);

    append(instr);
}

void Emitter::scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD,
                               reg_idx rA, reg_idx rB) {
    uint32_t instr = 0;

    uint32_t opcode = scalarArithmeticOpToAOpcode(op);
    instr |= (opcode << 25);

    // rD
    instr |= (rD.inner << 20);

    // rA
    instr |= (rA.inner << 15);

    // rB
    instr |= (rB.inner << 10);

    // arithmetic op
    instr |= scalarArithmeticOpToArithCode(op);

    append(instr);
}

void Emitter::floatArithmetic(isa::FloatArithmeticOp op, reg_idx rD,
                               reg_idx rA, reg_idx rB) {
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
                                vreg_idx vA, vreg_idx vB, reg_idx rD, 
                                reg_idx rA, u<4> mask, s<8> imm) {
    uint32_t instr = 0;

    uint32_t opcode = vectorArithmeticOpToAOpcode(op);
    instr |= (opcode << 25);

    switch(op) {
    case VectorArithmeticOp::Vdot:
        instr |= (rD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (vB << 10);
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
        instr |= mask.inner;
    case vectorArithmeticOp::Vindx:
        instr |= (rD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (imm.inner << 7); // this immediate is 2 bits, but the extras 
                                   // will flow into don't care spots so i didn't
                                   // adjust the size at all
    case vectorArithmeticOp::Vreduce:
        instr |= (rD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= mask.inner;
    case vectorArithmeticOp::Vsplat:
        instr |= (vD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= mask.inner;
    case vectorArithmeticOp::Vswizzle:
        instr |= (vD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (imm.inner << 7);
        instr |= mask.inner;
    case vectorArithmeticOp::Vcompsel:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (rB.inner << 10);
        instr |= (vB.inner << 5);
        instr |= mask.inner;
    // scalar+vector ops
    case vectorArithmeticOp::Vsadd:
    case vectorArithmeticOp::Vsmult:
    case vectorArithmeticOp::Vssub:
    case vectorArithmeticOp::Vsdiv:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (vA.inner << 10);
        instr |= mask.inner;
    // vector ops
    case vectorArithmeticOp::Vadd:
    case vectorArithmeticOp::Vsub:
    case vectorArithmeticOp::Vmult:
    case vectorArithmeticOp::Vdiv:
    case vectorArithmeticOp::Vmax:
    case vectorArithmeticOp::Vmin:
        instr |= (vD.inner << 20);
        instr |= (vA.inner << 15);
        instr |= (vB.inner << 10);
        instr |= mask.inner;
    default:
        panic("unsupported vector arith op");
    }

   append(instr);    
}

// matmul, writea, writeb, writec, readc, systolicstep
void Emitter::matrixMultiply(isa::matrixMultiplyOp op, vreg_idx vD, vreg_idx vA, vreg_idx vB,
                            u<3> idx, bool high) {

    uint32_t instr = 0;

    uint32_t opcode = matrixMultipyOpToAOpcode(op);
    instr |= (opcode << 25);

    switch(op) {
    case matrixMultipyOp::matmul:
    case matrixMultipyOp::systolicstep:
        break;
    case matrixMultipyOp::readC:
        instr |= (vD.inner << 20);
        instr |= (idx.inner << 17);
        if (high)
            instr |= (1<<16);
    case matrixMultipyOp::writeA:
    case matrixMultipyOp::writeB:
    case matrixMultipyOp::writeC:
        instr |= (idx << 20);
        instr |= (vA.inner << 15);
        instr |= (vB.inner << 10);
    default:
        panic("unsupported vector arith op");
    }

    append(instr);
}

