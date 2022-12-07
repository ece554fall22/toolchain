#pragma once

#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include <morph/decoder.h>
#include <morph/util.h>

#include "cpu.h"
#include "debugger.h"
#include "instructions.h"
#include "trace.h"

#define HANDLE_BR(suffix, cond, ...)                                           \
    case condition_t::cond:                                                    \
        instructions::b##cond##suffix(cpu, mem, ##__VA_ARGS__);                \
        break;

#define FORALL_CONDS(suffix, ...)                                              \
    HANDLE_BR(suffix, nz, ##__VA_ARGS__)                                       \
    HANDLE_BR(suffix, ez, ##__VA_ARGS__)                                       \
    HANDLE_BR(suffix, lz, ##__VA_ARGS__)                                       \
    HANDLE_BR(suffix, gz, ##__VA_ARGS__)                                       \
    HANDLE_BR(suffix, le, ##__VA_ARGS__)                                       \
    HANDLE_BR(suffix, ge, ##__VA_ARGS__)

class CPUInstructionProxy : public isa::InstructionVisitor {
  public:
    ~CPUInstructionProxy() override = default;
    CPUInstructionProxy(CPUState& cpu, MemSystem& mem, Debugger& dbg,
                        std::shared_ptr<Tracer> tracer)
        : cpu{cpu}, mem{mem}, dbg{dbg}, tracer{tracer} {}

    // misc
    void nop() override { instructions::nop(cpu, mem); }
    void halt() override { instructions::halt(cpu, mem); }
    void bkpt(bits<25> signal) override {
        fmt::print("{:#x}: breaking at BKPT (signaled {:#x})\n",
                   cpu.pc.getCurrentPC(), signal.inner);
        dbg.hitBreakpoint();
    }

    // J
    void jmp(s<25> imm) override {
        tracer->immInput(imm._sgn_inner());

        instructions::jmp(cpu, mem, imm);
    }
    void jal(s<25> imm) override {
        tracer->immInput(imm._sgn_inner());

        instructions::jal(cpu, mem, imm);

        tracer->scalarRegOutput(cpu, "rD", 31);
    }

    // JR
    void jmpr(reg_idx rA, s<20> imm) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm._sgn_inner());

        instructions::jmpr(cpu, mem, rA, imm);
    }
    void jalr(reg_idx rA, s<20> imm) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm._sgn_inner());

        instructions::jalr(cpu, mem, rA, imm);

        tracer->scalarRegOutput(cpu, "rD", 31);
    }

    // BI
    void branchimm(condition_t cond, s<22> imm) override {
        tracer->branchCondcode(cond);
        tracer->immInput(imm.inner);

        switch (cond) { FORALL_CONDS(i, imm) }

        tracer->controlFlow(cpu.pc);
    }
    // BR
    void branchreg(condition_t cond, reg_idx rA, s<17> imm) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm.inner);
        tracer->branchCondcode(cond);

        switch (cond) { FORALL_CONDS(r, rA, imm) }

        tracer->controlFlow(cpu.pc);
    }

    void lil(reg_idx rD, s<18> imm) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->immInput(imm._sgn_inner());

        instructions::lil(cpu, mem, rD, imm);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }
    void lih(reg_idx rD, s<18> imm) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->immInput(imm._sgn_inner());

        instructions::lih(cpu, mem, rD, imm);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm._sgn_inner());

        if (b36)
            instructions::ld36(cpu, mem, rD, rA, imm);
        else
            instructions::ld32(cpu, mem, rD, rA, imm);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);
        tracer->immInput(imm._sgn_inner());

        if (b36)
            instructions::st36(cpu, mem, rA, rB, imm);
        else
            instructions::st32(cpu, mem, rA, rB, imm);
    }

    void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                          isa::ScalarArithmeticOp op) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);

        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instructions::add(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::Sub:
            instructions::sub(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::Mul:
            instructions::mul(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::And:
            instructions::and_(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::Or:
            instructions::or_(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::Xor:
            instructions::xor_(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::Shr:
            instructions::shr(cpu, mem, rD, rA, rB);
            break;
        case isa::ScalarArithmeticOp::Shl:
            instructions::shl(cpu, mem, rD, rA, rB);
            break;
        default:
            panic("invalid op for immediate");
        }

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                   isa::ScalarArithmeticOp op) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm._sgn_inner());

        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instructions::addi(cpu, mem, rD, rA, imm);
            break;
        case isa::ScalarArithmeticOp::Sub:
            instructions::subi(cpu, mem, rD, rA, imm);
            break;
        case isa::ScalarArithmeticOp::And:
            instructions::andi(cpu, mem, rD, rA, imm);
            break;
        case isa::ScalarArithmeticOp::Or:
            instructions::ori(cpu, mem, rD, rA, imm);
            break;
        case isa::ScalarArithmeticOp::Xor:
            instructions::xori(cpu, mem, rD, rA, imm);
            break;
        case isa::ScalarArithmeticOp::Shr:
            instructions::shri(cpu, mem, rD, rA, imm);
            break;
        case isa::ScalarArithmeticOp::Shl:
            instructions::shli(cpu, mem, rD, rA, imm);
            break;
        default:
            panic("invalid op for immediate");
        }

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void arithmeticNot(reg_idx rD, reg_idx rA) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);

        instructions::not_(cpu, mem, rD, rA);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void vldi(vreg_idx vD, reg_idx rA, s<11> imm, vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm._sgn_inner());
        tracer->vectorMask(mask);

        instructions::vldi(cpu, mem, vD, rA, imm, mask);

        tracer->scalarRegOutput(cpu, "rA", rA);
    }

    void vsti(s<11> imm, reg_idx rA, vreg_idx vB, vmask_t mask) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->vectorRegInput(cpu, "vB", vB);
        tracer->immInput(imm._sgn_inner());
        tracer->vectorMask(mask);

        instructions::vsti(cpu, mem, rA, imm, vB, mask);

        tracer->scalarRegOutput(cpu, "rA", rA);
    }

    void vldr(vreg_idx vD, reg_idx rA, reg_idx rB, vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);
        tracer->vectorMask(mask);

        instructions::vldr(cpu, mem, vD, rA, rB, mask);

        tracer->scalarRegOutput(cpu, "rA", rA);
    }

    void vstr(reg_idx rA, reg_idx rB, vreg_idx vA, vmask_t mask) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorMask(mask);

        instructions::vstr(cpu, mem, rA, rB, vA, mask);

        tracer->scalarRegOutput(cpu, "rA", rA);
    }

    void floatArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                         isa::FloatArithmeticOp op) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);

        switch (op) {
        case isa::FloatArithmeticOp::Fadd:
            instructions::fadd(cpu, mem, rD, rA, rB);
            break;
        case isa::FloatArithmeticOp::Fsub:
            instructions::fsub(cpu, mem, rD, rA, rB);
            break;
        case isa::FloatArithmeticOp::Fmul:
            instructions::fmul(cpu, mem, rD, rA, rB);
            break;
        case isa::FloatArithmeticOp::Fdiv:
            instructions::fdiv(cpu, mem, rD, rA, rB);
            break;
        }

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    // -- float<->int conversions
    void ftoi(reg_idx rD, reg_idx rA) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);

        instructions::ftoi(cpu, mem, rD, rA);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void itof(reg_idx rD, reg_idx rA) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);

        instructions::itof(cpu, mem, rD, rA);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void cmp(reg_idx rA, reg_idx rB) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);

        instructions::cmp(cpu, mem, rA, rB);

        tracer->flagsWriteback(cpu.f);
    }

    void cmpdec(reg_idx rD, reg_idx rA, reg_idx rB) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);

        instructions::cmpdec(cpu, mem, rD, rA, rB);

        tracer->scalarRegOutput(cpu, "rD", rD);
        tracer->flagsWriteback(cpu.f);
    }

    void cmpinc(reg_idx rD, reg_idx rA, reg_idx rB) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);

        instructions::cmpinc(cpu, mem, rD, rA, rB);

        tracer->scalarRegOutput(cpu, "rD", rD);
        tracer->flagsWriteback(cpu.f);
    }

    void cmpI(reg_idx rA, s<20> imm) override {
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->immInput(imm._sgn_inner());

        instructions::cmpi(cpu, mem, rA, imm);

        tracer->flagsWriteback(cpu.f);
    }

    void vectorArithmetic(isa::LanewiseVectorOp op, vreg_idx vD, vreg_idx vA,
                          vreg_idx vB, vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorRegInput(cpu, "vB", vB);
        tracer->vectorMask(mask);

        switch (op) {
        case isa::LanewiseVectorOp::Add:
            instructions::vadd(cpu, mem, vD, vA, vB, mask);
            break;
        case isa::LanewiseVectorOp::Sub:
            instructions::vsub(cpu, mem, vD, vA, vB, mask);
            break;
        case isa::LanewiseVectorOp::Mul:
            instructions::vmul(cpu, mem, vD, vA, vB, mask);
            break;
        case isa::LanewiseVectorOp::Div:
            instructions::vdiv(cpu, mem, vD, vA, vB, mask);
            break;
        case isa::LanewiseVectorOp::Min:
            instructions::vmin(cpu, mem, vD, vA, vB, mask);
            break;
        case isa::LanewiseVectorOp::Max:
            instructions::vmax(cpu, mem, vD, vA, vB, mask);
            break;
        }

        tracer->vectorRegOutput(cpu, "vD", vD);
    }

    void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorRegInput(cpu, "vB", vB);

        instructions::vdot(cpu, mem, rD, vA, vB);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorRegInput(cpu, "vB", vB);

        instructions::vdota(cpu, mem, rD, rA, vA, vB);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void vidx(reg_idx rD, vreg_idx vA, vlaneidx_t imm) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->immInput(imm.raw());

        instructions::vidx(cpu, mem, rD, vA, imm);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void vreduce(reg_idx rD, vreg_idx vA, vmask_t mask) override {
        tracer->scalarRegInput(cpu, "rD", rD);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorMask(mask);

        instructions::vreduce(cpu, mem, rD, vA, mask);

        tracer->scalarRegOutput(cpu, "rD", rD);
    }

    void vsplat(vreg_idx vD, reg_idx rA, vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->vectorMask(mask);
        instructions::vsplat(cpu, mem, vD, rA, mask);

        tracer->vectorRegOutput(cpu, "vD", vD);
    }

    void vswizzle(vreg_idx vD, vreg_idx vA, vlaneidx_t i0, vlaneidx_t i1,
                  vlaneidx_t i2, vlaneidx_t i3, vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->swizzleInput(i0, i1, i2, i3);
        tracer->vectorMask(mask);

        instructions::vswizzle(cpu, mem, vD, vA, i0, i1, i2, i3, mask);

        tracer->vectorRegOutput(cpu, "vD", vD);
    }

    void vectorScalarArithmetic(isa::VectorScalarOp op, vreg_idx vD, reg_idx rA,
                                vreg_idx vB, vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->vectorRegInput(cpu, "vB", vB);
        tracer->vectorMask(mask);

        switch (op) {
        case isa::VectorScalarOp::Add:
            instructions::vsadd(cpu, mem, vD, rA, vB, mask);
            break;
        case isa::VectorScalarOp::Sub:
            instructions::vssub(cpu, mem, vD, rA, vB, mask);
            break;
        case isa::VectorScalarOp::Mul:
            instructions::vsmul(cpu, mem, vD, rA, vB, mask);
            break;
        case isa::VectorScalarOp::Div:
            instructions::vsdiv(cpu, mem, vD, rA, vB, mask);
            break;
        }

        tracer->vectorRegOutput(cpu, "vD", vD);
    }

    void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB,
              vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorRegInput(cpu, "vB", vB);
        tracer->vectorMask(mask);

        instructions::vsma(cpu, mem, vD, rA, vA, vB, mask);

        tracer->vectorRegOutput(cpu, "vD", vD);
    }

    void vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vA,
               vmask_t mask) override {
        tracer->vectorRegInput(cpu, "vD", vD);
        tracer->scalarRegInput(cpu, "rA", rA);
        tracer->scalarRegInput(cpu, "rB", rB);
        tracer->vectorRegInput(cpu, "vA", vA);
        tracer->vectorMask(mask);

        instructions::vcomp(cpu, mem, vD, rA, rB, vA, mask);

        tracer->vectorRegOutput(cpu, "vD", vD);
    }

    // -- systolic matrix stuff
    void matrixWrite(isa::MatrixWriteOp op, s<3> idx, vreg_idx vA,
                     vreg_idx vB) override {
        unimplemented();
    }

    void matmul() override { unimplemented(); }

    void systolicstep() override { unimplemented(); }

    void readC(vreg_idx vD, s<3> idx, bool high) override { unimplemented(); }

    // -- cache control
    void flushdirty() override { mem.flushDCacheDirty(); }

    void flushclean() override { mem.flushDCacheClean(); }

    void flushicache() override { mem.flushICache(); }

    void flushline(reg_idx rA, s<20> imm) override {
        mem.flushDCacheLine(cpu.r[rA] + imm._sgn_inner());
    }

    // -- atomics
    void fa(reg_idx rD, reg_idx rA, s<15> imm) override { unimplemented(); }

    void cmpx(reg_idx rD, reg_idx rA, s<15> imm) override { unimplemented(); }

    void wcsr(s<2> csr, reg_idx rA) override { unimplemented(); }
    void rcsr(s<2> csr, reg_idx rA) override { unimplemented(); }

  private:
    CPUState& cpu;
    MemSystem& mem;
    Debugger& dbg;
    std::shared_ptr<Tracer> tracer;
};
