// TODO(erin): replace all this manual bit twiddling

#include "morph/decoder.h"

using isa::InstructionVisitor;

void decodeJ(InstructionVisitor& visit, bits<32> instr);
void decodeJR(InstructionVisitor& visit, bits<32> instr);
void decodeBR(InstructionVisitor& visit, bits<32> instr);
void decodeBI(InstructionVisitor& visit, bits<32> instr);
void decodeLI(InstructionVisitor& visit, bits<32> instr);
void decodeML(InstructionVisitor& visit, bits<32> instr);
void decodeMS(InstructionVisitor& visit, bits<32> instr);
void decodeA(InstructionVisitor& visit, bits<32> instr);
void decodeAI(InstructionVisitor& visit, bits<32> instr);
void decodeCI(InstructionVisitor& visit, bits<32> instr);

void isa::decodeInstruction(InstructionVisitor& visit, bits<32> instr) {
    // crude decoder. our ISA is not compressed; we can just look
    // at the opcode field to understand what to do.
    auto opcode = instr.slice<31, 25>().inner;
    // fmt::print("opcode = {:#b}\n", opcode);
    switch (opcode) {
    case 0b0000'000: // halt
        return visit.halt();
    case 0b0000'001: // nop
        return visit.nop();

    // J-format
    case 0b0000'010: // jmp
    case 0b0000'011: // jal
        return decodeJ(visit, instr);

    // JR-format
    case 0b0000'100: // jmpr
    case 0b0000'101: // jalr
        return decodeJR(visit, instr);

    // BI-format: bxxi
    case 0b0000'110:
        return decodeBI(visit, instr);

    // BR-format: bxxr
    case 0b0000'111:
        return decodeBR(visit, instr);

    // LI-format: ldx
    case 0b0001'000: // lil
    case 0b0001'001: // lih
        unimplemented();
        return;

    // ML-format: ldxx
    case 0b0001'010: // ld32
    case 0b0001'011: // ld36
        return decodeBI(visit, instr);
    // MS-format: stxx
    case 0b0001'100: // st32
    case 0b0001'101: // st36
        unimplemented();
        return;

    // AI-format: immediate arithmetic
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
    case 0b0011'010: // compi
        unimplemented();
        return;

    // A-format: register arithmetic
    case 0b0011'011: // all of them
        return decodeA(visit, instr);

    // AN-format: not
    case 0b0011'100: // not
        unimplemented();
        return;

    // FA-format: floating arith
    case 0b0011'101: // all of them
        unimplemented();
        return;

    // C-format: cmp
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

void decodeJ(InstructionVisitor& visit, bits<32> instr) {
    auto imm = instr.slice<24, 0>();

    bool jal = instr.bit(25);
    if (jal)
        visit.jal(imm);
    else
        visit.jmp(imm);
}

void decodeJR(InstructionVisitor& visit, bits<32> instr) {
    auto rA = u<5>(instr.slice<19, 15>());
    auto imm = s<20>(instr.slice<24, 20>().concat(instr.slice<14, 0>()));

    bool jalr = instr.bit(25);
    if (jalr)
        visit.jalr(rA, imm);
    else
        visit.jmpr(rA, imm);
}

void decodeBR(InstructionVisitor& visit, bits<32> instr) {
    auto cond = condition_t(instr.slice<24, 22>().inner);
    auto rT = instr.slice<19, 15>();
    auto imm = s<17>(instr.slice<21, 20>().concat(instr.slice<14, 0>()));

    visit.branchreg(cond, rT, imm);
}

void decodeBI(InstructionVisitor& visit, bits<32> instr) {
    auto cond = condition_t(instr.slice<24, 22>().inner);
    auto imm = s<22>(instr.slice<21, 0>());

    visit.branchimm(cond, imm);
}

void decodeLI(InstructionVisitor& visit, bits<32> instr) {
    bool lo = instr.bit(25);

    auto rD = instr.slice<24, 20>();
    auto imm = instr.slice<17, 0>();

    if (lo)
        visit.lil(rD, imm);
    else
        visit.lih(rD, imm);
}

void decodeML(InstructionVisitor& visit, bits<32> instr) {
    bool b36 = instr.bit(25);

    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto imm = instr.slice<14, 0>();

    visit.ld(rD, rA, imm, b36);
}

void decodeMS(InstructionVisitor& visit, bits<32> instr) {
    bool b36 = instr.bit(25);

    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    auto imm = s<15>(instr.slice<24, 20>().concat(instr.slice<9, 0>()));

    visit.st(rA, rB, imm, b36);
}

void decodeA(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();

    auto op = instr.slice<3, 0>();

    visit.scalarArithmetic(rD, rA, rB, isa::scalarArithmeticOpFromArithCode(op));
}
