#include "cpu.h"
#include "varint.h"
// arbitrarily sized bitfields
// u<bits>
// s<bits>

namespace instructions {

void addi(CPUState& cpu, u<4> rD, u<4> rS, s<12> imm) {
    cpu.r[rD] = cpu.r[rS].asSigned() + imm.sext();
}

void add(CPUState& cpu, u<4> rD, u<4> rA, u<4> rB) {
    cpu.r[rD] = cpu.r[rA].asSigned() + cpu.r[rB].asSigned();
}

void cmp(CPUState& cpu, u<4> rD, u<4> rA, u<4> rB) {
    auto valA = cpu.r[rA].asSigned();
    auto valB = cpu.r[rB].asSigned();
    // ALU
    auto res = valA - valB;

    // compute flags
    cpu.f.zero = (res == 0);
    cpu.f.sign = (res < 0);
    cpu.f.overflow = (valA.sign() && valB.sign() && !res.sign()) ||
                     (!valA.sign() && !valB.sign() && res.sign());
}

} // namespace instructions
