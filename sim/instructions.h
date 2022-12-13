#pragma once

#include "cpu.h"
#include <Eigen/Dense>
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
        (cpu.r[rA].raw() << cpu.r[rB].slice<3, 0>().raw()) & bits<36>::mask;
}

// SHR
void shr(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, reg_idx rB) {
    cpu.r[rD].inner =
        (cpu.r[rA].raw() >> cpu.r[rB].slice<3, 0>().raw()) & bits<36>::mask;
}

void cmp(CPUState& cpu, MemSystem& mem, reg_idx rA, reg_idx rB) {
    auto valA = cpu.r[rA].asSigned();
    auto valB = cpu.r[rB].asSigned();
    // ALU
    s<36> res = valA - valB;

    // compute flags
    cpu.f.zero = (res == 0);
    cpu.f.sign = (res < 0);
    cpu.f.overflow = (valA.sign() && !valB.sign() && !res.sign()) ||
                     (!valA.sign() && valB.sign() && res.sign());
}

void cmpi(CPUState& cpu, MemSystem& mem, reg_idx rA, s<20> imm) {
    auto valA = cpu.r[rA].asSigned();
    auto valB = imm;
    // ALU
    auto res = valA - valB;

    // compute flags
    cpu.f.zero = (res == 0);
    cpu.f.sign = (res < 0);
    cpu.f.overflow = (valA.sign() && !valB.sign() && !res.sign()) ||
                     (!valA.sign() && valB.sign() && res.sign());
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
        auto valA = cpu.v[vA];                                                 \
        auto valB = cpu.v[vB];                                                 \
        _lane_apply(mask,                                                      \
                    [&](auto i) { cpu.v[vD][i] = valA[i] infixop valB[i]; });  \
    }

#define VEC_VS_BINOP(mnemonic, infixop)                                        \
    void mnemonic(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA,      \
                  vreg_idx vB, vmask_t mask) {                                 \
        auto val = bits2float(cpu.r[rA].slice<31, 0>());                       \
        auto vec = cpu.v[vB];                                                  \
        _lane_apply(mask, [&](auto i) { cpu.v[vD][i] = vec[i] infixop val; }); \
    }

#define VEC_EXTREMA(mnemonic, chooser)                                         \
    void mnemonic(CPUState& cpu, MemSystem& mem, vreg_idx vD, vreg_idx vA,     \
                  vreg_idx vB, vmask_t mask) {                                 \
        auto valA = cpu.v[vA];                                                 \
        auto valB = cpu.v[vB];                                                 \
        _lane_apply(                                                           \
            mask, [&](auto i) { cpu.v[vD][i] = chooser(valA[i], valB[i]); });  \
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
    auto valA = cpu.v[vA];
    _lane_apply(mask, [&](auto i) { cpu.v[vD][i] = valA[idxs[i].inner]; });
}

void vsma(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA, vreg_idx vA,
          vreg_idx vB, vmask_t mask) {
    float factor = bits2float(cpu.r[rA].slice<31, 0>());

    auto valA = cpu.v[vA];
    auto valB = cpu.v[vB];
    _lane_apply(mask,
                [&](auto i) { cpu.v[vD][i] = valA[i] * factor + valB[i]; });
}

void vcomp(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA, reg_idx rB,
           vreg_idx vB, vmask_t mask) {
    float a = bits2float(cpu.r[rA].slice<31, 0>());
    float b = bits2float(cpu.r[rB].slice<31, 0>());

    auto valB = cpu.v[vB];
    _lane_apply(mask, [&](auto i) { cpu.v[vD][i] = (valB[i] > 0.f) ? a : b; });
}

// -- scalar memory instructions
void ld32(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = mem.read32(addr.raw());
    cpu.r[rD].inner = val;
}

void ld36(CPUState& cpu, MemSystem& mem, reg_idx rD, reg_idx rA, s<15> imm) {
    u<36> addr = cpu.r[rA] + imm;
    auto val = mem.read36(addr.raw());
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

// -- vector memory instructions
void vldi(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA, s<11> imm,
          vmask_t mask) {
    u<36> addr = cpu.r[rA];

    auto val = mem.readVec(addr.raw());
    cpu.v[vD] = val;

    addr += imm._sgn_inner() * 0x10; // in vector increments
    cpu.r[rA] = addr;
}

void vsti(CPUState& cpu, MemSystem& mem, reg_idx rA, s<11> imm, vreg_idx vB,
          vmask_t mask) {
    u<36> addr = cpu.r[rA];

    mem.write(addr.raw(), cpu.v[vB]);

    addr += imm._sgn_inner() * 0x10; // in vector increments
    cpu.r[rA] = addr;
}

// vldr vD, [rA+=rB]
void vldr(CPUState& cpu, MemSystem& mem, vreg_idx vD, reg_idx rA, reg_idx rB,
          vmask_t mask) {
    u<36> addr = cpu.r[rA];

    auto val = mem.readVec(addr.raw());
    cpu.v[vD] = val;

    addr += cpu.r[rB]._sgn_inner();
    cpu.r[rA] = addr;
}

// vstr [rA+=rB], vA
void vstr(CPUState& cpu, MemSystem& mem, reg_idx rA, reg_idx rB, vreg_idx vA,
          vmask_t mask) {
    u<36> addr = cpu.r[rA];

    mem.write(addr.raw(), cpu.v[vA]);

    addr += cpu.r[rB]._sgn_inner();
    cpu.r[rA] = addr;
}

/************************************************************************/
/*******************************-- JUMPS ********************************/
/************************************************************************/

// JMP
void jmp(CPUState& cpu, MemSystem& mem, s<25> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken(true);
}

void jal(CPUState& cpu, MemSystem& mem, s<25> imm) {
    cpu.r[31] = cpu.pc.peekNotTaken();         // link
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4); // and jump
    cpu.pc.setTaken(true);
}

void jmpr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.pc.setTakenPC(cpu.r[rT].inner + imm._sgn_inner() * 4); // and jump
    cpu.pc.setTaken(true);
}

void jalr(CPUState& cpu, MemSystem& mem, reg_idx rT, s<20> imm) {
    cpu.r[31] = cpu.pc.peekNotTaken();                         // link
    cpu.pc.setTakenPC(cpu.r[rT].inner + imm._sgn_inner() * 4); // and jump
    cpu.pc.setTaken(true);
}

/************************************************************************/
/************************-- Conditional Branches ************************/
/************************************************************************/

// BNZR
void bnzr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    cpu.pc.setTakenPC(cpu.r[rA].inner + imm._sgn_inner());
    cpu.pc.setTaken(!cpu.f.zero);
}

// BEZR
void bezr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    cpu.pc.setTakenPC(cpu.r[rA] + imm);
    cpu.pc.setTaken(cpu.f.zero);
}

// BLZR
void blzr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    cpu.pc.setTakenPC(cpu.r[rA] + imm);
    cpu.pc.setTaken(cpu.f.sign ^ cpu.f.overflow);
}

// BGZR
void bgzr(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    cpu.pc.setTakenPC(cpu.r[rA] + imm);
    cpu.pc.setTaken(!(cpu.f.sign ^ cpu.f.overflow) & !cpu.f.zero);
}

// BLER
void bler(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    cpu.pc.setTakenPC(cpu.r[rA] + imm);
    cpu.pc.setTaken((cpu.f.sign ^ cpu.f.overflow) || cpu.f.zero);
}

// BGER
void bger(CPUState& cpu, MemSystem& mem, reg_idx rA, s<17> imm) {
    cpu.pc.setTakenPC(cpu.r[rA] + imm);
    cpu.pc.setTaken(!(cpu.f.sign || cpu.f.overflow));
}

/** Relative to current PC */

// BNZI
void bnzi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken(!cpu.f.zero);
}

// BEZI
void bezi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken(cpu.f.zero);
}

// BLZI
void blzi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken(cpu.f.sign ^ cpu.f.overflow);
}

// BGZI
void bgzi(CPUState& cpu, MemSystem& mem, s<22> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken(!(cpu.f.sign ^ cpu.f.overflow) & !cpu.f.zero);
}

// BLEI
void blei(CPUState& cpu, MemSystem& mem, s<22> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken((cpu.f.sign ^ cpu.f.overflow) || cpu.f.zero);
}

// BGEI
void bgei(CPUState& cpu, MemSystem& mem, s<22> imm) {
    cpu.pc.addToTakenPC(imm._sgn_inner() * 4);
    cpu.pc.setTaken(!(cpu.f.sign || cpu.f.overflow));
}

void _cpuWriteToMatrix(CPUState& cpu, MatrixUnit::Matrix& mat, vreg_idx vA,
                       vreg_idx vB, u<3> row) {
    for (size_t i = 0; i < 4; i++)
        mat(row.raw(), i) = cpu.v[vA][i];
    for (size_t i = 0; i < 4; i++)
        mat(row.raw(), i + 4) = cpu.v[vB][i];
}

/**
 * Write the rows of A
 * Assuming vA holds the higher bits and vB the lower bits of
 * Memory
 *
 * Note: Data is also written from the zeroth row (which is at the
 * to like in standard matrices)
 */
void writeA(CPUState& cpu, MemSystem& mem, vreg_idx vA, vreg_idx vB, u<3> row) {
    _cpuWriteToMatrix(cpu, cpu.matUnit.A, vA, vB, row);
}

/**
 * Write the rows of B
 * Assuming vA holds the higher bits and vB the lower bits of
 * Memory
 *
 * Note: Data is also written from the zeroth row (which is at the
 * to like in standard matrices)
 */
void writeB(CPUState& cpu, MemSystem& mem, vreg_idx vA, vreg_idx vB, u<3> row) {
    _cpuWriteToMatrix(cpu, cpu.matUnit.B, vA, vB, row);
}

void writeC(CPUState& cpu, MemSystem& mem, vreg_idx vA, vreg_idx vB, u<3> row) {
    _cpuWriteToMatrix(cpu, cpu.matUnit.C, vA, vB, row);
}

// Matrix Multiply unit
void matmul(CPUState& cpu, MemSystem& mem) {
    cpu.matUnit.C.noalias() += cpu.matUnit.A * cpu.matUnit.B;
}

void systolicStep(CPUState& cpu, MemSystem& mem) {
    cpu.matUnit.systolicCycleCt += 1;
    if (cpu.matUnit.systolicCycleCt >= MatrixUnit::SYSTOLIC_CYCLES) {
        cpu.matUnit.systolicCycleCt = 0;
        matmul(cpu, mem);
    }
}

void readC(CPUState& cpu, MemSystem& mem, vreg_idx vD, u<3> row, bool hi) {
    size_t offs = 0;
    if (hi) {
        offs += 4;
    }

    for (size_t i = 0; i < 4; i++) {
        cpu.v[vD][i] = cpu.matUnit.C(row.raw(), offs + i);
    }
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
