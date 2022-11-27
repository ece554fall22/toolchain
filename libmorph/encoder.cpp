#include "morph/encoder.h"

using isa::ScalarArithmeticOp;
using isa::FloatArithmeticOp;
using isa::VectorArithmeticOp;
using isa::MatrixMultiplyOp;
using isa::LoadStoreOp;
using isa::FlushCacheOp;
using isa::CsrOp;
using isa::FloatIntConversionOp;
using isa::ConcurrencyOp;
using isa::BranchCompareOp;
using isa::HaltNopOp;


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
    case ScalarArithmeticOp::Not:
        return 0b0011100;
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
    switch(op) {
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

uint32_t loadStoreOpToAOpcode(LoadStoreOp op) {
    switch(op) {
    case LoadStoreOp::Lih:
        return 0b0001000;
    case LoadStoreOp::Lil:
        return 0b0001001;
    case LoadStoreOp::Ld32:
        return 0b0001010;
    case LoadStoreOp::Ld36:
        return 0b0001011;
    case LoadStoreOp::St32:
        return 0b0001100;
    case LoadStoreOp::St36:
        return 0b0001101;
    case LoadStoreOp::Vldi:
        return 0b0001110;
    case LoadStoreOp::Vsti:
        return 0b0010000;
    case LoadStoreOp::Vldr:
        return 0b0010001;
    case LoadStoreOp::Vstr:
        return 0b0010010;
    default:
        panic("unsupported load/store op");
        return 0;
    }
}

uint32_t flushCacheOpToAOpcode(FlushCacheOp op) {
    switch(op) {
    case FlushCacheOp::Flushdirty:
        return 0b0111101;
    case FlushCacheOp::Flushclean:
        return 0b0111110;
    case FlushCacheOp::Flushicache:
        return 0b0111111;
    case FlushCacheOp::Flushline:
        return 0b1000000;
    default:
        panic("unsupported cache flush op");
        return 0;
    }
}

uint32_t branchCompareOpToAOpcode(BranchCompareOp op) {
    switch(op) {
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

    if (op != ScalarArithmeticOp::Not) {
        // rB
        instr |= (rB.inner << 10);

        // arithmetic op
        instr |= scalarArithmeticOpToArithCode(op);
    }

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
                                reg_idx rA, reg_idx rB, u<4> mask, s<8> imm) {
    uint32_t instr = 0;

    uint32_t opcode = vectorArithmeticOpToAOpcode(op);
    instr |= (opcode << 25);

    switch(op) {
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
        instr |= ((imm.inner & 0b11111111) << 7); // this immediate is 2 bits, but the extras 
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
void Emitter::matrixMultiply(isa::MatrixMultiplyOp op, vreg_idx vD, vreg_idx vA, vreg_idx vB,
                            u<3> idx, bool high) {

    uint32_t instr = 0;

    uint32_t opcode = matrixMultiplyOpToAOpcode(op);
    instr |= (opcode << 25);

    switch(op) {
    case MatrixMultiplyOp::Matmul:
    case MatrixMultiplyOp::Systolicstep:
        break;
    case MatrixMultiplyOp::ReadC:
        instr |= (vD.inner << 20);
        instr |= ((idx.inner & 0b111) << 17);
        if (high)
            instr |= (1<<16);
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

// lih, lil, ld32, ld36, st32, st36, vldi, vsti, vldr, vstr
void Emitter::loadStore(isa::LoadStoreOp op, vreg_idx vD, vreg_idx vA, reg_idx rD, reg_idx rA,
                        reg_idx rB, u<18> imm, u<4> mask) {
    
    uint32_t instr = 0;
    uint32_t opcode = loadStoreOpToAOpcode(op);
    instr |= (opcode << 25);

    switch(op) {
    case LoadStoreOp::Lih:
    case LoadStoreOp::Lil:
        instr |= (rD.inner << 20);
        instr |= (imm.inner & 0b111111111111111111);
    case LoadStoreOp::Ld32:
    case LoadStoreOp::Ld36:
        instr |= (rD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (imm.inner & 0b11111111111111);
    case LoadStoreOp::St32:
    case LoadStoreOp::St36:
        instr |= ((imm.inner & 0b111110000000000) << 20);
        instr |= (rA.inner << 15);
        instr |= (rB.inner << 10);
        instr |= (imm.inner & 0b1111111111);
    case LoadStoreOp::Vldi:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= ((imm.inner & 0b11111111111) << 4);
        instr |= (mask.inner & 0b1111);
    case LoadStoreOp::Vsti:
        instr |= ((imm.inner & 0b11111000000) << 20);
        instr |= (rA.inner << 15);
        instr |= (vA.inner << 10);
        instr |= ((imm.inner & 0b111111) << 4);
        instr |= (mask.inner & 0b1111);
    case LoadStoreOp::Vldr:
        instr |= (vD.inner << 20);
        instr |= (rA.inner << 15);
        instr |= (rB.inner << 10);
        instr |= (mask.inner & 0b1111);
    case LoadStoreOp::Vstr:
        instr |= (rA.inner << 15);
        instr |= (rB.inner << 10);
        instr |= (vA.inner << 5);
        instr |= (mask.inner & 0b1111);
    default:
        panic("unsupported load/store op");
        return;
    }
    append(instr);                 
}

// flushdirty, flushclean, flushicache, flushline
void Emitter::flushCache(isa::FlushCacheOp op, u<25> imm) {

    uint32_t instr = 0;
    uint32_t opcode = flushCacheOpToAOpcode(op);
    instr |= (opcode << 25);
    if (op == FlushCacheOp::Flushline) {
        auto immHigh = (imm.inner >> 15) & 0b1111111111;
        auto immLow = imm.inner & 0b111111111111111;
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
    }
    else if (op == CsrOp::Rcsr) {
        instr |= (0b0111000 << 25);
    }
    else {
        panic("unsupported csr op");
        return;
    }
    instr |= (csrNum.inner & 0b11) << 23;
    
    append(instr);
}

// ftoi, itof
void Emitter::floatIntConv(isa::FloatIntConversionOp op, reg_idx rD, reg_idx rA) {

    uint32_t instr = 0;

    if (op == FloatIntConversionOp::Ftoi) {
        instr |= (0b0110111 << 25);
    }
    else if (op == FloatIntConversionOp::Itof) {
        instr |= (0b0111000);
    }
    else {
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
        instr |= (imm.inner & 0b111111111111111);
    }
    else if (op == ConcurrencyOp::Cmpx) {
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

    switch(op) {
    case BranchCompareOp::Bi:
        instr |= ((btx.inner & 0b111) << 22);
        instr |= imm.inner & 0b1111111111111111111111; // TODO: yeah i hate this (should be 22 1s)
    case BranchCompareOp::Br:
        instr |= ((btx.inner & 0b111) << 22);
        instr |= ((imm.inner >> 15) & 0b11) << 20;
        instr |= rA.inner << 15;
        instr |= (imm.inner & 0b111111111111111);
    case BranchCompareOp::Cmpi:
        instr |= ((imm.inner >> 15) & 0b11111) << 20;
        instr |= rA.inner << 15;
        instr |= (imm.inner & 0b111111111111111);
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

// halt, nop
void Emitter::haltNop(isa::HaltNopOp op) {
    uint32_t instr = 0;
    if (op == HaltNopOp::Halt) {
        instr |= (0b0000000 << 25);
    }
    else if (op == HaltNopOp::Nop) {
        instr |= (0b0000001 << 25);
    }
    else {
        panic("unsupported halt/nop");
        return;
    }

    append(instr);
}


