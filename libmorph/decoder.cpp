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
void decodeBkpt(InstructionVisitor& visit, bits<32> instr);
void decodeCmpI(InstructionVisitor& visit, bits<32> instr);
void decodeNot(InstructionVisitor& visit, bits<32> instr);
void decodeFA(InstructionVisitor& visit, bits<32> instr);
void decodeCmp(InstructionVisitor& visit, bits<32> instr);
void decodeVA(InstructionVisitor& visit, bits<32> instr);
void decodeVdot(InstructionVisitor& visit, bits<32> instr);
void decodeVdota(InstructionVisitor& visit, bits<32> instr);
void decodeVidx(InstructionVisitor& visit, bits<32> instr);
void decodeVreduce(InstructionVisitor& visit, bits<32> instr);
void decodeVsplat(InstructionVisitor& visit, bits<32> instr);
void decodeVswizzle(InstructionVisitor& visit, bits<32> instr);
void decodeVSA(InstructionVisitor& visit, bits<32> instr);
void decodeVsma(InstructionVisitor& visit, bits<32> instr);
void decodeMatrixWrite(InstructionVisitor& visit, bits<32> instr);
void decodeReadC(InstructionVisitor& visit, bits<32> instr);
void decodeVcomp(InstructionVisitor& visit, bits<32> instr);
void decodeFa(InstructionVisitor& visit, bits<32> instr);
void decodeCmpx(InstructionVisitor& visit, bits<32> instr);
void decodeFtoi(InstructionVisitor& visit, bits<32> instr);
void decodeItof(InstructionVisitor& visit, bits<32> instr);
void decodeWcsr(InstructionVisitor& visit, bits<32> instr);
void decodeRcsr(InstructionVisitor& visit, bits<32> instr);
void decodeCmpdec(InstructionVisitor& visit, bits<32> instr);
void decodeCmpinc(InstructionVisitor& visit, bits<32> instr);
void decodeVldi(InstructionVisitor& visit, bits<32> instr);
void decodeVsti(InstructionVisitor& visit, bits<32> instr);
void decodeVldr(InstructionVisitor& visit, bits<32> instr);
void decodeVstr(InstructionVisitor& visit, bits<32> instr);
void decodeFlushline(InstructionVisitor& visit, bits<32> instr);

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
        return decodeLI(visit, instr);

    // ML-format: ldxx
    case 0b0001'010: // ld32
    case 0b0001'011: // ld36
        return decodeML(visit, instr);
    // MS-format: stxx
    case 0b0001'100: // st32
    case 0b0001'101: // st36
        return decodeMS(visit, instr);

    // Vldi
    case 0b0001'110:
        return decodeVldi(visit, instr);

    // Vsti
    case 0b0010'000:
        return decodeVsti(visit, instr);

    // Vldr
    case 0b0010'001:
        return decodeVldr(visit, instr);

    // Vstr
    case 0b0010'010:
        return decodeVstr(visit, instr);

    // AI-format: immediate arithmetic
    case 0b0010'011: // addi
    case 0b0010'100: // subi
    case 0b0010'101: // andi
    case 0b0010'110: // ori
    case 0b0010'111: // xori
    case 0b0011'000: // shli
    case 0b0011'001: // shri
        return decodeAI(visit, instr);

    // CI-format: imm compare
    case 0b0011'010: // compi
        return decodeCmpI(visit, instr);
        return;

    // A-format: register arithmetic
    case 0b0011'011: // all of them
        return decodeA(visit, instr);

    // AN-format: not
    case 0b0011'100: // not
        return decodeNot(visit, instr);

    // FA-format: floating arith
    case 0b0011'101: // all of them
        return decodeFA(visit, instr);

    // C-format: cmp
    case 0b0011'110: // cmp
        return decodeCmp(visit, instr);

    // vector arith
    case 0b0011'111:
    case 0b0100'000:
    case 0b0100'001:
    case 0b0100'010:
        return decodeVA(visit, instr);

    // vdot
    case 0b0100'011:
        return decodeVdot(visit, instr);

    // vdota
    case 0b0100'100:
        return decodeVdota(visit, instr);

    // vidx
    case 0b0100'101:
        return decodeVidx(visit, instr);

    // vreduce
    case 0b0100'110:
        return decodeVreduce(visit, instr);

    // vsplat
    case 0b0100'111:
        return decodeVsplat(visit, instr);

    // vswizzle
    case 0b0101'000:
        return decodeVswizzle(visit, instr);

    // vector scalar arith ops
    case 0b0101'001:
    case 0b0101'010:
    case 0b0101'011:
    case 0b0101'100:
        return decodeVSA(visit, instr);

    // vsma
    case 0b0101'101:
        return decodeVsma(visit, instr);

    // matrix write ops
    case 0b0101'110:
    case 0b0101'111:
    case 0b0110'000:
        return decodeMatrixWrite(visit, instr);

    // matmul
    case 0b0110'001:
        return visit.matmul();

    // systolicstep
    case 0b0110'011:
        return visit.systolicstep();

    // readc
    case 0b0110'010:
        return decodeReadC(visit, instr);

    // vcomp
    case 0b0110'110:
        return decodeVcomp(visit, instr);

    // ftoi
    case 0b0110'111:
        return decodeFtoi(visit, instr);

    // itof
    case 0b0111'000:
        return decodeItof(visit, instr);

    // wcsr
    case 0b0111'001:
        return decodeWcsr(visit, instr);

    // rcsr
    case 0b0111'010:
        return decodeRcsr(visit, instr);

    // fa
    case 0b0111'011:
        return decodeFa(visit, instr);

    // cmpx
    case 0b0111'100:
        return decodeCmpx(visit, instr);

    // memory management
    case 0b0111'101: // flushdirty
        return visit.flushdirty();
    case 0b0111'110: // flushclean
        return visit.flushclean();
    case 0b0111'111: // flushicache
        return visit.flushicache();

    case 0b1000'000: // flushline
        return decodeFlushline(visit, instr);

    // cmpdec
    case 0b1000'001:
        return decodeCmpdec(visit, instr);

    // cmpinc
    case 0b1000'010:
        return decodeCmpinc(visit, instr);

    case 0b1010'101: // bkpt
        return decodeBkpt(visit, instr);

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

void decodeVldi(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto imm = instr.slice<14, 4>();
    auto mask = instr.slice<3, 0>();
    visit.vldi(vD, rA, imm, mask);
}

void decodeVsti(InstructionVisitor& visit, bits<32> instr) {
    auto imm = instr.slice<24, 20>().concat(instr.slice<9, 4>());
    auto rA = instr.slice<19, 15>();
    auto vB = instr.slice<14, 10>();
    auto mask = instr.slice<3, 0>();
    visit.vsti(imm, rA, vB, mask);
}

void decodeVldr(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    auto mask = instr.slice<3, 0>();
    visit.vldr(vD, rA, rB, mask);
}

void decodeVstr(InstructionVisitor& visit, bits<32> instr) {
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    auto vA = instr.slice<9, 5>();
    auto mask = instr.slice<3, 0>();
    visit.vstr(rA, rB, vA, mask);
}

void decodeA(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();

    auto op = instr.slice<3, 0>();

    visit.scalarArithmetic(rD, rA, rB,
                           isa::scalarArithmeticOpFromArithCode(op));
}

void decodeAI(InstructionVisitor& visit, bits<32> instr) {
    auto op = isa::scalarArithmeticOpFromAIOpcode(instr.slice<31, 25>());

    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto imm = instr.slice<14, 0>();

    visit.scalarArithmeticImmediate(rD, rA, imm, op);
}

void decodeBkpt(InstructionVisitor& visit, bits<32> instr) {
    auto imm = instr.slice<24, 0>();
    visit.bkpt(imm);
}

void decodeCmpI(InstructionVisitor& visit, bits<32> instr) {
    auto imm = s<20>(instr.slice<24, 20>().concat(instr.slice<14, 0>()));
    auto rA = instr.slice<19, 15>();
    visit.cmpI(rA, imm);
}

void decodeNot(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    visit.arithmeticNot(rD, rA);
}

void decodeFA(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    auto op = isa::floatArithmeticOpFromArithCode(instr.slice<2, 0>());
    visit.floatArithmetic(rD, rA, rB, op);
}

void decodeCmp(InstructionVisitor& visit, bits<32> instr) {
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    visit.cmp(rA, rB);
}

void decodeVA(InstructionVisitor& visit, bits<32> instr) {
    auto op = isa::vectorArithmeticOpFromOpcode(instr.slice<31, 25>());
    auto vD = instr.slice<24, 20>();
    auto vA = instr.slice<19, 15>();
    auto vB = instr.slice<14, 10>();
    auto mask = instr.slice<3, 0>();
    visit.vectorArithmetic(op, vD, vA, vB, mask);
}

void decodeVdot(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto vA = instr.slice<19, 15>();
    auto vB = instr.slice<14, 10>();
    visit.vdot(rD, vA, vB);
}

void decodeVdota(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto vA = instr.slice<14, 10>();
    auto vB = instr.slice<9, 5>();
    visit.vdota(rD, rA, vA, vB);
}

void decodeVidx(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto vA = instr.slice<19, 15>();
    auto imm = instr.slice<8, 7>();
    visit.vidx(rD, vA, imm);
}

void decodeVreduce(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto vA = instr.slice<19, 15>();
    auto mask = instr.slice<3, 0>();
    visit.vreduce(rD, vA, mask);
}

void decodeVsplat(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto mask = instr.slice<3, 0>();
    visit.vsplat(vD, rA, mask);
}

void decodeVswizzle(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto vA = instr.slice<19, 15>();
    auto i4 = instr.slice<14, 13>();
    auto i3 = instr.slice<12, 11>();
    auto i2 = instr.slice<10, 9>();
    auto i1 = instr.slice<8, 7>();
    auto mask = instr.slice<3, 0>();
    visit.vswizzle(vD, vA, i1, i2, i3, i4, mask);
}

void decodeVSA(InstructionVisitor& visit, bits<32> instr) {
    auto op = isa::vectorScalarArithmeticOpFromOpcode(instr.slice<31, 25>());
    auto vD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto vB = instr.slice<14, 10>();
    auto mask = instr.slice<3, 0>();
    visit.vectorScalarArithmetic(op, vD, rA, vB, mask);
}

void decodeVsma(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto vA = instr.slice<14, 10>();
    auto vB = instr.slice<9, 5>();
    auto mask = instr.slice<3, 0>();
    visit.vsma(vD, rA, vA, vB, mask);
}

void decodeMatrixWrite(InstructionVisitor& visit, bits<32> instr) {
    auto op = isa::matrixWriteOpFromOpcode(instr.slice<31, 25>());
    auto idx = instr.slice<22, 20>();
    auto vA = instr.slice<19, 15>();
    auto vB = instr.slice<14, 10>();
    visit.matrixWrite(op, idx, vA, vB);
}

void decodeReadC(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto idx = instr.slice<19, 17>();
    bool high = instr.bit(16);
    visit.readC(vD, idx, high);
}

void decodeVcomp(InstructionVisitor& visit, bits<32> instr) {
    auto vD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    auto vB = instr.slice<9, 5>();
    auto mask = instr.slice<3, 0>();
    visit.vcomp(vD, rA, rB, vB, mask);
}

void decodeFa(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto imm = instr.slice<14, 0>();
    visit.fa(rD, rA, imm);
}

void decodeCmpx(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto imm = instr.slice<14, 0>();
    visit.cmpx(rD, rA, imm);
}

void decodeFtoi(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    visit.ftoi(rD, rA);
}

void decodeItof(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    visit.itof(rD, rA);
}

void decodeWcsr(InstructionVisitor& visit, bits<32> instr) {
    auto csr = instr.slice<24, 23>();
    auto rA = instr.slice<19, 15>();
    visit.wcsr(csr, rA);
}

void decodeRcsr(InstructionVisitor& visit, bits<32> instr) {
    auto csr = instr.slice<24, 23>();
    auto rA = instr.slice<19, 15>();
    visit.rcsr(csr, rA);
}

void decodeCmpdec(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    visit.cmpdec(rD, rA, rB);
}

void decodeCmpinc(InstructionVisitor& visit, bits<32> instr) {
    auto rD = instr.slice<24, 20>();
    auto rA = instr.slice<19, 15>();
    auto rB = instr.slice<14, 10>();
    visit.cmpinc(rD, rA, rB);
}

void decodeFlushline(InstructionVisitor& visit, bits<32> instr) {
    auto rA = u<5>(instr.slice<19, 15>());
    auto imm = s<20>(instr.slice<24, 20>().concat(instr.slice<14, 0>()));

    return visit.flushline(rA, imm);
}
