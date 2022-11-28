#include "cpu.h"
#include <morph/varint.h>
// arbitrarily sized bitfields
// u<bits>
// s<bits>

namespace instructions {

// -- lmao
void nop(CPUState& cpu, MemSystem& mem) {}
void halt(CPUState& cpu, MemSystem& mem) { exit(1); } // TODO

// -- load immediate
void lil(CPUState& cpu, MemSystem& mem, reg_idx rD, s<18> imm) {
    auto mask = 0b000000000000000000111111111111111111;
    cpu.r[rD].inner = (cpu.r[rD].inner & ~mask) | imm.inner;
}

void lih(CPUState& cpu, MemSystem& mem, reg_idx rD, s<18> imm) {
    auto mask = 0b111111111111111111000000000000000000;
    cpu.r[rD].inner = (cpu.r[rD].inner & ~mask) | (imm.inner << 18);
}

/************************************************************************/
/************************-- scalar alu instruction **********************/
/************************************************************************/

// ADDI
void addi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD] = (cpu.r[rA].asSigned() + imm).asUnsigned();
}

// SUBI
void subi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD] = (cpu.r[rA].asSigned() - imm).asUnsigned();
}

// ANDI
void andi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD] = (cpu.r[rA] & imm.asUnsigned());
}

// ORI
void ori(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD] = (cpu.r[rA] | imm.asUnsigned());
}

// XORI
void xori(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD] = (cpu.r[rA] ^ imm.asUnsigned());
}

// SHLI
void shli(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD].inner = (cpu.r[rA].inner << imm._sgn_inner()) & bits<36>::mask;
}

// SHRI
void shri(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    cpu.r[rD].inner = (cpu.r[rA].inner >> imm._sgn_inner()) & bits<36>::mask;
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
    cpu.r[rD] =
        cpu.r[rA].asSigned().truncMult<36>(cpu.r[rB].asSigned()).asUnsigned();
}

// AND
void and_(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA] & cpu.r[rB]);
}

// OR
void or_(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA] | cpu.r[rB]);
}

// XOR
void xor_(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA] ^ cpu.r[rB]);
}

// SHL
void shl(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD].inner =
        (cpu.r[rA].inner << cpu.r[rB]._sgn_inner()) & bits<36>::mask;
}

// SHR
void shr(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD].inner =
        (cpu.r[rA].inner >> cpu.r[rB]._sgn_inner()) & bits<36>::mask;
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
using vmask_t = bits<4>;
using velem_idx_t = bits<2>;

template <typename F> inline void _lane_apply(vmask_t mask, F fn) {
    for (size_t lane = 0; lane < N_LANES; lane++) {
        if (mask.bit(lane))
            fn(lane);
    }
}

#define VEC_BINOP(mnemonic, infixop)                                           \
    void mnemonic(CPUState& cpu, MemSystem& mem, vreg_idx vD, vreg_idx vA,     \
                  vreg_idx vB, vmask_t mask) {                                 \
        _lane_apply(mask, [&](auto i) {                                        \
            cpu.v[vD][i] = cpu.v[vA][i] infixop cpu.v[vB][i];                  \
        });                                                                    \
    }

#define VEC_VS_BINOP(mnemonic, infixop)                                        \
    void mnemonic(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA,      \
                  vreg_idx vB, vmask_t mask) {                                 \
        auto val = bits2float(cpu.r[rA].slice<31, 0>());                       \
        _lane_apply(mask,                                                      \
                    [&](auto i) { cpu.v[vD][i] = cpu.v[vB][i] infixop val; }); \
    }

VEC_BINOP(vadd, +);
VEC_BINOP(vsub, -);
VEC_BINOP(vmul, *);
VEC_BINOP(vdiv, /);

VEC_VS_BINOP(vsadd, +);
VEC_VS_BINOP(vssub, -);
VEC_VS_BINOP(vsmul, *);
VEC_VS_BINOP(vsdiv, /);

void vdot(CPUState& cpu, MemSystem& mem, reg_idx rD, vreg_idx vA, vreg_idx vB,
          vmask_t mask) {

    // compute dot
    float acc = 0.f;
    _lane_apply(mask, [&](auto i) { acc += cpu.v[vA][i] * cpu.v[vB][i]; });

    // dump to rD
    cpu.r[rD].inner =
        float2bits(acc)
            .inner; // okay because float2bits . :: -> bits<32> ⊂ bits<36>
}

void vdota(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, vreg_idx vA,
           vreg_idx vB, vmask_t mask) {

    // load rA to accumulator
    float acc = bits2float(cpu.r[rA].slice<31, 0>());

    // compute dot
    _lane_apply(mask, [&](auto i) { acc += cpu.v[vA][i] * cpu.v[vB][i]; });

    // dump to rD
    cpu.r[rD].inner =
        float2bits(acc)
            .inner; // okay because float2bits . :: -> bits<32> ⊂ bits<36>
}

void vidx(CPUState& cpu, MemSystem& mem, reg_idx rD, vreg_idx vA,
          velem_idx_t idx) {
    cpu.r[rD].inner = float2bits(cpu.v[vA][idx.inner]).inner; // reasoning ibid
}

void vreduce(CPUState& cpu, MemSystem& mem, reg_idx rD, vreg_idx vA,
             vmask_t mask) {
    float acc = 0.f;

    _lane_apply(mask, [&](auto i) { acc += cpu.v[vA][i]; });

    cpu.r[rD].inner = float2bits(acc).inner; // reasoning ibid
}

void vsplat(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA,
            vmask_t mask) {
    float val = bits2float(cpu.r[rA].slice<31, 0>());

    _lane_apply(mask, [&](auto i) { cpu.v[vD][i] = val; });
}

void vswizzle(CPUState& cpu, MemSystem& mem, vreg_idx vD, vreg_idx vA,
              velem_idx_t idxs[4], vmask_t mask) {
    _lane_apply(mask, [&](auto i) { cpu.v[vD][i] = cpu.v[vA][idxs[i].inner]; });
}

// -- scalar memory instructions
void ld32(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = mem.read32(addr.inner);
    cpu.r[rD].inner = val;
}

void ld36(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = mem.read36(addr.inner);
    cpu.r[rD].inner = val;
}

void st32(CPUState& cpu, MemSystem& mem, reg_idx rA, reg_idx rB, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = cpu.r[rB].slice<31, 0>();
    mem.write(addr, val);
}

void st36(CPUState& cpu, MemSystem& mem, reg_idx rA, reg_idx rB, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = cpu.r[rB];
    mem.write(addr, val);
}

/************************************************************************/
/*******************************-- JUMPS ********************************/
/************************************************************************/

// JMP
void jmp(CPUState& cpu, MemSystem& mem, s<25> imm) {
    cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

void jal(CPUState& cpu, MemSystem& mem, s<25> imm) {
    cpu.r[31] = cpu.pc.getCurrentPC();        // link
    cpu.pc.addToNextPC(imm._sgn_inner() * 4); // and jump
}

void jmpr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.r[31] = cpu.pc.getCurrentPC();                        // link
    cpu.pc.setNextPC(cpu.r[rT].inner + imm._sgn_inner() * 4); // and jump
}

void jalr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.r[31] = cpu.pc.getCurrentPC();                        // link
    cpu.pc.setNextPC(cpu.r[rT].inner + imm._sgn_inner() * 4); // and jump
}

/************************************************************************/
/************************-- Conditional Branches ************************/
/************************************************************************/

// BNZR
void bnzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rT].inner + imm._sgn_inner());
}

// BEZR
void bezr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rT] + imm);
}

// BLZR
void blzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign)
        cpu.pc.setNextPC(cpu.r[rT] + imm);
}

// BGZR
void bgzr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign)
        cpu.pc.setNextPC(cpu.r[rT] + imm);
}

// BLER
void bler(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign | cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rT] + imm);
}

// BGER
void bger(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign | cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rT] + imm);
}

/** Relative to current PC */

// BNZI
void bnzi(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BEZI
void bezi(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BLZI
void blzi(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BGZI
void bgzi(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BLEI
void blei(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (cpu.f.sign || cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BGEI
void bgei(CPUState& cpu, MemSystem& mem, reg_idx rT, s<15> imm) {
    if (!cpu.f.sign || cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// -- specials: cache control
// functions in MemSystem might just be stubs, but we still want to be able to
// execute these ops

void flushicache(CPUState& cpu, MemSystem& mem) { mem.flushICache(); }

void flushdirty(CPUState& cpu, MemSystem& mem) { mem.flushDCacheDirty(); }

void flushclean(CPUState& cpu, MemSystem& mem) { mem.flushDCacheClean(); }

void flushline(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    uint64_t addr = cpu.r[rT].inner + imm._sgn_inner();
    mem.flushDCacheLine(addr);
}

} // namespace instructions
