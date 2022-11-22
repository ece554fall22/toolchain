// TODO(erin): replace all this manual bit twiddling

#include "morph/decoder.h"

#include <fmt/core.h>

void isa::decodeInstruction(InstructionVisitor& visit, instr_t instr) {
    // crude decoder. our ISA is not compressed; we can just look
    // at the opcode field to understand what to do.
    auto opcode = instr.slice<31, 25>().inner;
    // fmt::print("opcode = {:#b}\n", opcode);
    switch (opcode) {
    case 0b0000'000: // halt
        visit.halt();
        return;
    case 0b0000'001: // nop
        visit.nop();
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
        decodeJR(visit, instr);
        return;

    // BI-format: bxxi
    // 0000 110X XXii iiii iiii iiii iiii iiii
    case 0b0000'110:
        decodeBI(visit, instr);
        return;

    // BR-format: bxxr
    // 0000 111X XXTT TTTi iiii iiii iiii iiii
    case 0b0000'111:
        decodeBR(visit, instr);
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

void isa::decodeJ(InstructionVisitor& visit, instr_t instr) {
    auto imm = instr.slice<24, 0>();

    bool jal = instr.bit(25);
    if (jal)
        visit.jal(imm);
    else
        visit.jmp(imm);
}

void isa::decodeJR(InstructionVisitor& visit, instr_t instr) {
    auto rA = u<5>(instr.slice<19, 15>());
    auto imm = s<20>(instr.slice<24, 20>().concat(instr.slice<14, 0>()));

    bool jalr = instr.bit(25);
    if (jalr)
        visit.jalr(rA, imm);
    else
        visit.jmpr(rA, imm);
}

void isa::decodeBR(InstructionVisitor& visit, instr_t instr) {
    auto cond = condition_t(instr.slice<24, 22>().inner);
    auto rT = instr.slice<19, 15>();
    auto imm = s<17>(instr.slice<21, 20>().concat(instr.slice<14, 0>()));

    visit.branchreg(cond, rT, imm);
}

void isa::decodeBI(InstructionVisitor& visit, instr_t instr) {
    auto cond = condition_t(instr.slice<24, 22>().inner);
    auto imm = s<22>(instr.slice<21, 0>());

    visit.branchimm(cond, imm);
}
