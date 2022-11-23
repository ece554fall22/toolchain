#include "cpu.h"
#include <morph/varint.h>
// arbitrarily sized bitfields
// u<bits>
// s<bits>

namespace instructions {

/************************************************************************/
/************************-- scalar alu instruction **********************/
/************************************************************************/

// ADDI 
void addi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() + imm).asUnsigned();
}

// SUBI
void subi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() - imm).asUnsigned();
}

// ANDI
void andi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() & imm).asUnsigned();
}

// ORI
void ori(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() | imm).asUnsigned();
}

// XORI
void xori(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() ^ imm).asUnsigned();
}

// SHLI
void shli(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() >> imm).asUnsigned();
}

// SHRI
void shri(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rS, s<15> imm) {
    cpu.r[rD] = (cpu.r[rS].asSigned() << imm).asUnsigned();
}

// ADD
void add(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() + cpu.r[rB].asSigned()).asUnsigned();
}

// SUB
void sub(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() + cpu.r[rB].asSigned()).asUnsigned();
}

// MULT
void mult(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() * cpu.r[rB].asSigned()).asUnsigned();
}

// AND
void and(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() & cpu.r[rB].asSigned()).asUnsigned();
}

// OR
void or(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() | cpu.r[rB].asSigned()).asUnsigned();
}

// XOR
void xor(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() ^ cpu.r[rB].asSigned()).asUnsigned();
}

// SHL
void shl(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() >> cpu.r[rB].asSigned()).asUnsigned();
}

// SHR
void shr(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() << cpu.r[rB].asSigned()).asUnsigned();
}

void cmp(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
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

// -- vector instructions
constexpr size_t N_LANES = 4;

void vmul(CPUState& cpu, MemSystem& mem, vreg_idx vD, vreg_idx vA, vreg_idx vB,
          u<4> mask) {
    for (size_t lane = 0; lane < N_LANES; lane++) {
        if (mask.bit(lane))
            cpu.v[vD][lane] = cpu.v[vA][lane] * cpu.v[vB][lane];
    }
}

// -- scalar memory instructions
void st32(CPUState& cpu, MemSystem& mem, reg_idx rS, reg_idx rA, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = cpu.r[rS];
    mem.write(addr, val);
}

// -- jumps
void jalr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.r[31] = cpu.pc.get();         // link
    cpu.pc.redirect(cpu.r[rT] + imm); // and jump
}

/************************************************************************/
/*******************************-- JUMPS ********************************/
/************************************************************************/

// JMPR
void jmpr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.pc.redirect(cpu.r[rT] + (imm << 2)); // and jump (imm * 4)
}

// JALR
void jalr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.r[31] = cpu.pc.get();         // link
    cpu.pc.redirect(cpu.r[rT] + (imm << 2)); // and jump (imm * 4)
}

// JMP
void jmp(CPUState& cpu, MemSystem& mem, reg_idx rT, s<25> imm) {
    addr_t nxt_pc = cpu.pc.get() + 4; 
    cpu.pc.redirect(nxt_pc + (imm << 2)); // imm * 4
}

// JAL
void jal(CPUState& cpu, MemSystem& mem, reg_idx rT, s<25> imm) {
    addr_t curr_pc = cpu.pc.get();
    addr_t nxt_pc = curr_pc + 4; 
    cpu.r[31] = curr_pc;                // link
    cpu.pc.redirect(nxt_pc + (imm << 2)); // and jump (imm * 4)
}

/************************************************************************/
/************************-- Conditional Branches ************************/
/************************************************************************/

// BNZR
void bnzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.zero)
        cpu.pc.redirect(cpu.r[rT] + imm);
}

// BEZR
void bezr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.zero)
        cpu.pc.redirect(cpu.r[rT] + imm);
}

// BLZR
void blzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign)
        cpu.pc.redirect(cpu.r[rT] + imm);
}

// BGZR
void bgzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign)
        cpu.pc.redirect(cpu.r[rT] + imm);
}

// BLER
void bler(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign | cpu.f.zero)
        cpu.pc.redirect(cpu.r[rT] + imm);
}

// BGER
void bger(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign | cpu.f.zero)
        cpu.pc.redirect(cpu.r[rT] + imm);
}

/** Relative to current PC */

// BNZI
void bnzi(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.zero)
        cpu.pc.redirect(cpu.pc.get() + imm);
}

// BEZI
void bezr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.zero)
        cpu.pc.redirect(cpu.pc.get() + imm);
}

// BLZI
void blzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign)
        cpu.pc.redirect(cpu.pc.get() + imm);
}

// BGZI
void bgzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign)
        cpu.pc.redirect(cpu.pc.get() + imm);
}

// BLEI
void bler(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign | cpu.f.zero)
        cpu.pc.redirect(cpu.pc.get() + imm);
}

// BGEI
void bgei(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign | cpu.f.zero)
        cpu.pc.redirect(cpu.pc.get() + imm);
}

// -- specials: cache control
// functions in MemSystem might just be stubs, but we still want to be able to
// execute these ops

void flushicache(CPUState& cpu, MemSystem& mem) { mem.flushICache(); }

void flushdirty(CPUState& cpu, MemSystem& mem) { mem.flushDCacheDirty(); }

void flushclean(CPUState& cpu, MemSystem& mem) { mem.flushDCacheClean(); }

void flushline(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    addr_t addr = cpu.r[rT] + imm;
    mem.flushDCacheLine(addr);
}

} // namespace instructions
