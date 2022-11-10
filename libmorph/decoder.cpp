#include "decoder.h"

#include <fmt/core.h>

void isa::decodeInstruction(InstructionVisitor& visit, uint32_t instr) {
    // crude decoder. our ISA is not compressed; we can just look
    // at the opcode field to understand what to do.
    uint32_t opcode = (instr & 0b1111'1110'0000'0000'0000'0000'0000'0000) >> 25;
    // fmt::print("opcode = {:#b}\n", opcode);
    switch (opcode) {
    case 0b0000'000: // nop
        visit.nop();
        return;
    case 0b0000'001: // halt
        visit.halt();
        return;

    // J-format
    // 0000 01Xi iiii iiii iiii iiii iiii iiii
    case 0b0000'010: // jmp
    case 0b0000'011: // jal
        decodeJ(visit, instr);
        return;

    // JR-format
    // 0000 10XT TTTT iiii iiii iiii iiii iiii
    case 0b0000'100: // jmpr
    case 0b0000'101: // jalr
        unimplemented();
        return;

    // BI-format: bxxi
    // 0000 110X XXii iiii iiii iiii iiii iiii
    case 0b0000'110:
        unimplemented();
        return;

    // B-format: bxxr
    // 0000 111X XXTT TTTi iiii iiii iiii iiii
    case 0b0000'111:
        unimplemented();
        return;

    // LI-format: ldx
    // 0001 00XD DDDD ZZii iiii iiii iiii iiii
    case 0b0001'000: // lil
    case 0b0001'001: // lih
        unimplemented();
        return;

    // M-format: ldxx, stxx
    // 0001 XXXD DDDD SSSS Siii iiii iiii iiii
    case 0b0001'010: // ld32
    case 0b0001'011: // ld36
    case 0b0001'100: // st32
    case 0b0001'101: // st36
        unimplemented();
        return;

    // AI-format: immediate arithmetic
    // 001X XXXD DDDD SSSS Siii iiii iiii iiii
    case 0b0010'011: // addi
    case 0b0010'100: // subi
    case 0b0010'101: // andi
    case 0b0010'110: // ori
    case 0b0010'111: // xori
    case 0b0011'000: // shli
    case 0b0011'001: // shri
        unimplemented();
        return;

    // CI-format: imm compare
    // 0011 010T TTTT iiii iiii iiii iiii iiii
    case 0b0011'010: // compi
        unimplemented();
        return;

    // A-format: register arithmetic
    // 0011 011D DDDD SSSS STTT TTZZ ZZZZ ZXXX
    case 0b0011'011: // all of them
        unimplemented();
        return;
    // AN-format: not
    // 0011 100D DDDD TTTT TZZZ ZZZZ ZZZZ ZZZZ
    case 0b0011'100: // not
        unimplemented();
        return;

    // FA-format: floating arith
    // 0011 101D DDDD SSSS STTT TTZZ ZZZZ ZZXX
    case 0b0011'101: // all of them
        unimplemented();
        return;

    // C-format: cmp
    // 0011 110T TTTT SSSS SZZZ ZZZZ ZZZZ ZZZZ
    case 0b0011'110: // cmp
        unimplemented();
        return;

    // memory management
    case 0b0111'101: // flushdirty
    case 0b0111'110: // flushclean
    case 0b0111'111: // flushicache

    case 0b1000'000: // flushline

        unimplemented();
        return;

    default:
        // TODO: proper except or something
        unimplemented();
        return;
    }
}

void isa::decodeJ(InstructionVisitor& visit, uint32_t instr) {
    uint32_t maskImm = (1 << 25) - 1;

    auto imm = s<25>::fromBits(instr & maskImm);

    bool jal = instr & 0b0000'0010'0000'0000'0000'0000'0000'0000;
    if (jal)
        visit.jal(imm);
    else
        visit.jmp(imm);
}