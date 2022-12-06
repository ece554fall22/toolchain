#pragma once

#include "cpu.h"
#include <morph/varint.h>
// arbitrarily sized bitfields
// u<bits>
// s<bits>

namespace instructions {

// -- lmao
void nop(CPUState& cpu, MemSystem& mem) {}
void halt(CPUState& cpu, MemSystem& mem) { cpu.halt(); }

// -- load immediate
void lil(CPUState& cpu, MemSystem& mem, reg_idx rD, s<18> imm) {
    auto mask = 0b000000000000000000111111111111111111;
    cpu.r[rD].inner = (cpu.r[rD].inner & ~mask) | imm.raw();
}

void lih(CPUState& cpu, MemSystem& mem, reg_idx rD, s<18> imm) {
    auto mask = 0b111111111111111111000000000000000000;
    cpu.r[rD].inner = (cpu.r[rD].inner & ~mask) | (imm.raw() << 18);
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

void cmpi(CPUState& cpu, MemSystem& mem, reg_idx rA, s<20> imm) {
    auto valA = cpu.r[rA].asSigned();
    auto valB = imm;
    // ALU
    auto res = valA - valB;

    // compute flags
    cpu.f.zero = (res == 0);
    cpu.f.sign = (res < 0);
    cpu.f.overflow = (valA.sign() && valB.sign() && !res.sign()) ||
                     (!valA.sign() && !valB.sign() && res.sign());
}

// ADD
void add(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() + cpu.r[rB].asSigned()).asUnsigned();
}

// SUB
void sub(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] = (cpu.r[rA].asSigned() - cpu.r[rB].asSigned()).asUnsigned();
}

// MUL
void mul(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD] =
        cpu.r[rA].asSigned().truncMult<36>(cpu.r[rB].asSigned()).asUnsigned();
}

// #define BITOP_RRR(mnemonic, infixop) {

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
        (cpu.r[rA].raw() << cpu.r[rB]._sgn_inner()) & bits<36>::mask;
}

// SHR
void shr(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD].inner =
        (cpu.r[rA].raw() >> cpu.r[rB]._sgn_inner()) & bits<36>::mask;
}

void cmp(CPUState& cpu, MemSystem& mem, reg_idx rA, reg_idx rB) {
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

// can just view this like it's microcoded lol
void cmpdec(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cmp(cpu, mem, rD, rA);
    sub(cpu, mem, rD, rD, rB);
}

void cmpinc(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cmp(cpu, mem, rD, rA);
    add(cpu, mem, rD, rD, rB);
}

void not_(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA) {
    cpu.r[rD] = ~cpu.r[rD];
}

// -- scalar float instructions
#define SCALAR_FLOAT_BINOP(mnemonic, infixop)                                  \
    void mnemonic(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA,       \
                  reg_idx rB) {                                                \
        float a = bits2float(cpu.r[rA].slice<31, 0>());                        \
        float b = bits2float(cpu.r[rB].slice<31, 0>());                        \
        float d = a infixop b;                                                 \
        cpu.r[rD].inner = float2bits(d).inner;                                 \
    }

SCALAR_FLOAT_BINOP(fadd, +);
SCALAR_FLOAT_BINOP(fsub, -);
SCALAR_FLOAT_BINOP(fmul, *);
SCALAR_FLOAT_BINOP(fdiv, /);

void itof(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA) {
    float val = (float)cpu.r[rA].asSigned()._sgn_inner();
    cpu.r[rD].inner = float2bits(val).inner;
}

void ftoi(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA) {
    float val = bits2float(cpu.r[rA].slice<31, 0>());

    // trunc
    auto ival = static_cast<int64_t>(val);

    cpu.r[rD] = s<36>(ival).asUnsigned();
}

// -- vector instructions
template <typename F> inline void _lane_apply(F fn) {
    for (size_t lane = 0; lane < N_LANES; lane++) {
        fn(lane);
    }
}

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

#define VEC_EXTREMA(mnemonic, chooser)                                         \
    void mnemonic(CPUState& cpu, MemSystem& mem, vreg_idx vD, vreg_idx vA,     \
                  vreg_idx vB, vmask_t mask) {                                 \
        _lane_apply(mask, [&](auto i) {                                        \
            cpu.v[vD][i] = chooser(cpu.v[vA][i], cpu.v[vB][i]);                \
        });                                                                    \
    }

VEC_BINOP(vadd, +);
VEC_BINOP(vsub, -);
VEC_BINOP(vmul, *);
VEC_BINOP(vdiv, /);

VEC_VS_BINOP(vsadd, +);
VEC_VS_BINOP(vssub, -);
VEC_VS_BINOP(vsmul, *);
VEC_VS_BINOP(vsdiv, /);

VEC_EXTREMA(vmax, std::max);
VEC_EXTREMA(vmin, std::min);

void vdot(CPUState& cpu, MemSystem& mem, reg_idx rD, vreg_idx vA, vreg_idx vB) {

    // compute dot
    float acc = 0.f;
    _lane_apply([&](auto i) { acc += cpu.v[vA][i] * cpu.v[vB][i]; });

    // dump to rD
    cpu.r[rD].inner =
        float2bits(acc)
            .inner; // okay because float2bits . :: -> bits<32> ⊂ bits<36>
}

void vdota(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, vreg_idx vA,
           vreg_idx vB) {

    // load rA to accumulator
    float acc = bits2float(cpu.r[rA].slice<31, 0>());

    // compute dot
    _lane_apply([&](auto i) { acc += cpu.v[vA][i] * cpu.v[vB][i]; });

    // dump to rD
    cpu.r[rD].inner =
        float2bits(acc)
            .inner; // okay because float2bits . :: -> bits<32> ⊂ bits<36>
}

void vidx(CPUState& cpu, MemSystem& mem, reg_idx rD, vreg_idx vA,
          vlaneidx_t idx) {
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
              vlaneidx_t i0, vlaneidx_t i1, vlaneidx_t i2, vlaneidx_t i3,
              vmask_t mask) {
    vlaneidx_t idxs[4] = {i0, i1, i2, i3};
    _lane_apply(mask, [&](auto i) { cpu.v[vD][i] = cpu.v[vA][idxs[i].inner]; });
}

void vsma(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA, vreg_idx vA,
          vreg_idx vB, vmask_t mask) {
    float factor = bits2float(cpu.r[rA].slice<31, 0>());

    _lane_apply(mask, [&](auto i) {
        cpu.v[vD][i] = cpu.v[vA][i] * factor + cpu.v[vB][i];
    });
}

void vcomp(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA, reg_idx rB,
           vreg_idx vB, vmask_t mask) {
    float a = bits2float(cpu.r[rA].slice<31, 0>());
    float b = bits2float(cpu.r[rB].slice<31, 0>());

    _lane_apply(mask,
                [&](auto i) { cpu.v[vD][i] = (cpu.v[vB][i] > 0.f) ? a : b; });
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
    cpu.r[31] = cpu.pc.peekNotTaken();        // link
    cpu.pc.addToNextPC(imm._sgn_inner() * 4); // and jump
}

void jmpr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.pc.setNextPC(cpu.r[rT].inner + imm._sgn_inner() * 4); // and jump
}

void jalr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.r[31] = cpu.pc.peekNotTaken();                        // link
    cpu.pc.setNextPC(cpu.r[rT].inner + imm._sgn_inner() * 4); // and jump
}

/************************************************************************/
/************************-- Conditional Branches ************************/
/************************************************************************/

// BNZR
void bnzr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    if (!cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rA].inner + imm._sgn_inner());
}

// BEZR
void bezr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    if (cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rA] + imm);
}

// BLZR
void blzr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    if (cpu.f.sign)
        cpu.pc.setNextPC(cpu.r[rA] + imm);
}

// BGZR
void bgzr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    if (!cpu.f.sign)
        cpu.pc.setNextPC(cpu.r[rA] + imm);
}

// BLER
void bler(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    if (cpu.f.sign | cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rA] + imm);
}

// BGER
void bger(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    if (!cpu.f.sign | cpu.f.zero)
        cpu.pc.setNextPC(cpu.r[rA] + imm);
}

/** Relative to current PC */

// BNZI
void bnzi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    if (!cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BEZI
void bezi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    if (cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BLZI
void blzi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    if (cpu.f.sign)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BGZI
void bgzi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    if (!cpu.f.sign)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BLEI
void blei(CPUState& cpu, MemSystem& mem, s<22> imm) {
    if (cpu.f.sign || cpu.f.zero)
        cpu.pc.addToNextPC(imm._sgn_inner() * 4);
}

// BGEI
void bgei(CPUState& cpu, MemSystem& mem, s<22> imm) {
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
