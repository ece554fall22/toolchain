#pragma once

#include <filesystem>
#include <iostream>

#include <argparse/argparse.hpp>

#include <morph/decoder.h>
#include <morph/util.h>

#include "cpu.h"
#include "instructions.h"

struct CPUInstructionProxy : public isa::InstructionVisitor {
    virtual ~CPUInstructionProxy() = default;
    CPUInstructionProxy(auto& cpu, auto& mem) : cpu{cpu}, mem{mem} {}
    CPUInstructionProxy(auto&& cpu, auto&& mem) = delete;
    // misc
    void nop() override { instructions::nop(cpu, mem); }
    void halt() override { instructions::halt(cpu, mem); }
    void bkpt(bits<25> signal) override {
        fmt::print(
            "\n\nBREAKPOINT BREAKPOINT : {:#x} : BREAKPOINT BREAKPOINT\n\n",
            signal.inner);
    }

    // J
    void jmp(s<25> imm) override { instructions::jmp(cpu, mem, imm); }
    void jal(s<25> imm) override { instructions::jal(cpu, mem, imm); }

    // JR
    void jmpr(reg_idx rA, s<20> imm) override {
        instructions::jmpr(cpu, mem, rA, imm);
    }
    void jalr(reg_idx rA, s<20> imm) override {
        instructions::jalr(cpu, mem, rA, imm);
    }

    // BI
    void branchimm(condition_t cond, s<22> imm) override {
        todo("branch instruction proxies");
    }
    // BR
    void branchreg(condition_t cond, reg_idx rA, s<17> imm) override {
        todo("branch instruction proxies");
    }

    void lil(reg_idx rD, s<18> imm) override {
        instructions::lil(cpu, mem, rD, imm);
    }
    void lih(reg_idx rD, s<18> imm) override {
        instructions::lih(cpu, mem, rD, imm);
    }

    void ld(reg_idx rD, reg_idx rA, s<15> imm, bool b36) override {
        if (b36)
            instructions::ld36(cpu, mem, rD, rA, imm);
        else
            instructions::ld32(cpu, mem, rD, rA, imm);
    }

    void st(reg_idx rA, reg_idx rB, s<15> imm, bool b36) override {
        if (b36)
            instructions::st36(cpu, mem, rA, rB, imm);
        else
            instructions::st32(cpu, mem, rA, rB, imm);
    }

    void scalarArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                          isa::ScalarArithmeticOp op) override {
        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instructions::add(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Sub:
            instructions::sub(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Mul:
            instructions::mul(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::And:
            instructions::and_(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Or:
            instructions::or_(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Xor:
            instructions::xor_(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Shr:
            instructions::shr(cpu, mem, rD, rA, rB);
            return;
        case isa::ScalarArithmeticOp::Shl:
            instructions::shl(cpu, mem, rD, rA, rB);
            return;
        }
    }

    void scalarArithmeticImmediate(reg_idx rD, reg_idx rA, s<15> imm,
                                   isa::ScalarArithmeticOp op) override {
        switch (op) {
        case isa::ScalarArithmeticOp::Add:
            instructions::addi(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Sub:
            instructions::subi(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::And:
            instructions::andi(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Or:
            instructions::ori(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Xor:
            instructions::xori(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Shr:
            instructions::shri(cpu, mem, rD, rA, imm);
            return;
        case isa::ScalarArithmeticOp::Shl:
            instructions::shli(cpu, mem, rD, rA, imm);
            return;
        default:
            panic("invalid op for immedate");
        }
    }

    void vldi(vreg_idx vD, reg_idx rA, s<11> imm, s<4> mask) override {}

    void vsti(s<11> imm, reg_idx rA, reg_idx vB, s<4> mask) override {}

    void vldr(vreg_idx vD, reg_idx rA, reg_idx rB, s<4> mask) override {}

    void vstr(reg_idx rA, reg_idx rB, vreg_idx vA, s<4> mask) override {}

    void cmpI(reg_idx rA, s<20> imm) override {}

    void arithmeticNot(reg_idx rD, reg_idx rA) override {}

    void floatArithmetic(reg_idx rD, reg_idx rA, reg_idx rB,
                         isa::FloatArithmeticOp op) override {}

    void cmp(reg_idx rA, reg_idx rB) override {}

    void vectorArithmetic(isa::VectorArithmeticOp op, vreg_idx vD, vreg_idx vA,
                          vreg_idx vB, s<4> mask) override {}

    void vdot(reg_idx rD, vreg_idx vA, vreg_idx vB) override {}

    void vdota(reg_idx rD, reg_idx rA, vreg_idx vA, vreg_idx vB) override {}

    void vindx(reg_idx rD, vreg_idx vA, s<2> imm) override {}

    void vreduce(reg_idx rD, vreg_idx vA, s<4> mask) override {}

    void vsplat(vreg_idx vD, reg_idx rA, s<4> mask) override {}

    void vswizzle(vreg_idx vD, vreg_idx vA, s<2> i1, s<2> i2, s<2> i3, s<2> i4,
                  s<4> mask) override {}

    void vectorScalarArithmetic(isa::VectorScalarArithmeticOp op, vreg_idx vD,
                                reg_idx rA, vreg_idx vB, s<4> mask) override {}

    void vsma(vreg_idx vD, reg_idx rA, vreg_idx vA, vreg_idx vB,
              s<4> mask) override {}

    void matrixWrite(isa::MatrixWriteOp op, s<3> idx, vreg_idx vA,
                     vreg_idx vB) override {}

    void matmul() override {}

    void systolicstep() override {}

    void readC(vreg_idx vD, s<3> idx, bool high) override {}

    void vcomp(vreg_idx vD, reg_idx rA, reg_idx rB, vreg_idx vB,
               s<4> mask) override {}

    void flushdirty() override { mem.flushDCacheDirty(); }

    void flushclean() override { mem.flushDCacheClean(); }

    void flushicache() override { mem.flushICache(); }

    void flushline(reg_idx rA, s<20> imm) override {
        mem.flushDCacheLine(cpu.r[rA] + imm._sgn_inner());
    }

    void fa(reg_idx rD, reg_idx rA, s<15> imm) override {}

    void cmpx(reg_idx rD, reg_idx rA, s<15> imm) override {}

    void ftoi(reg_idx rD, reg_idx rA) override {}

    void itof(reg_idx rD, reg_idx rA) override {}

    void wcsr(s<2> csr, reg_idx rA) override {}

    void rcsr(s<2> csr, reg_idx rA) override {}

    void cmpdec(reg_idx rD, reg_idx rA, reg_idx rB) override {}

    void cmpinc(reg_idx rD, reg_idx rA, reg_idx rB) override {}

  private:
    CPUState& cpu;
    MemSystem& mem;
};
