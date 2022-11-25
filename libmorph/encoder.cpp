#include "morph/encoder.h"

using isa::ScalarArithmeticOp;


// TODO: halt, nop, bi, br, lih, lil, ld32, ld36, st32, st36, vldi, vsti, vldr, vstr, not, cmp, vadd,
// vsub, vmult, vdiv, vdot, vdota, vindx, vreduce, vswizzle, vsplat, vsadd, vsmult, vssub, vsdiv, vsma, writea, writeb, writec,
// matmul, readc, systolicstep, vmax, vmin, vcompsel, ftoi, itof, wcsr, rcsr, fa, cmpx, flushdirty, flushclean, flushicache,
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
    case FloatArithmeticOp::Fmult:
        return 0b010;
    case FloatArithmeticOp::Fdiv:
        return 0b011;
    default:
        panic("unsupported scalar arith op for A format");
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
    instr |= (imm.inner & 0b111111111111111);

    append(instr);
}

void isa::Emitter::scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD,
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
