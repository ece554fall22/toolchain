#include "morph/encoder.h"

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

void Emitter::arithmeticImmediate(isa::ScalarArithmeticOp op, reg_idx rD,
                                  reg_idx rA, s<15> imm) {
    uint32_t instr = 0;
    uint32_t opcode = isa::scalarArithmeticOpToAIOpcode(op);

    // opcode
    instr |= (opcode << 25);

    // rD
    instr |= (rD.inner << 20);

    // rA
    instr |= (rA.inner << 15);

    // imm
    instr |= imm.inner;

    append(instr);
}
