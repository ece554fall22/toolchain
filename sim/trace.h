#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>

#include "cpu.h"
#include <morph/ty.h>

struct Tracer {
    virtual ~Tracer() = default;

    /**
     * Called when entering a fresh instruction.
     * @param pc
     * @param ir
     */
    virtual void begin(uint64_t pc, uint64_t ir) = 0;

    /**
     * Called when the current instruction completes, dumping it to the
     * tracefile ostream.
     */
    virtual void end() = 0;

    //    virtual void rA(reg_idx rA) = 0;
    //    virtual void rB(reg_idx rB) = 0;
    //    virtual void rD(reg_idx rD) = 0;
    virtual void immInput(int64_t imm) = 0;
    virtual void scalarRegInput(CPUState& cpu, const char* name, reg_idx r) = 0;
    virtual void vectorRegInput(CPUState& cpu, const char* name,
                                vreg_idx r) = 0;
    virtual void writebackScalarReg(CPUState& cpu, const char* name,
                                    reg_idx r) = 0;

    virtual void memWrite(uint64_t addr, u<32> val) = 0;
    virtual void memWrite(uint64_t addr, u<36> val) = 0;
    virtual void memWrite(uint64_t addr, f32x4 val) = 0;
    virtual void memRead32(uint64_t addr, uint32_t val) = 0;
    virtual void memRead36(uint64_t addr, uint64_t val) = 0;

    virtual void condcode(condition_t cond) = 0;
};

struct NullTracer : public Tracer {
    virtual ~NullTracer() = default;

    void begin(uint64_t pc, uint64_t ir) override {}

    void end() override {}

    void scalarRegInput(CPUState& cpu, const char* name, reg_idx r) override {}

    void vectorRegInput(CPUState& cpu, const char* name, vreg_idx r) override {}

    void writebackScalarReg(CPUState& cpu, const char* name,
                            reg_idx r) override {}

    void immInput(int64_t imm) override {}

    void memWrite(uint64_t addr, u<32> val) override {}

    void memWrite(uint64_t addr, u<36> val) override {}

    void memWrite(uint64_t addr, f32x4 val) override {}

    void memRead32(uint64_t addr, uint32_t val) override {}

    void memRead36(uint64_t addr, uint64_t val) override {}

    void condcode(condition_t cond) override {}
};

// struct PreExReg {
//     enum class Slot {
//         rA,
//         rB,
//         rD,
//         vA,
//         vB,
//         vD,
//     };
//
//     Slot slot;
//     bool vector;
//     uint32_t idx;
//     std::variant<u<36>, f32x4> val;
// };

struct InstructionTrace {
    uint64_t pc;
    uint64_t ir;
    std::vector<std::string> inputs;
    std::optional<condition_t> condcode;
    std::optional<std::string> scalarRegOutput;
    std::optional<std::string> vectorRegOutput;

    //    std::optional<std::string> scalarMemWrite;
    //    std::optional<std::string> scalarMemRead;
    //    std::optional<std::string> vectorMemWrite;
    //    std::optional<std::string> vectorMemRead;

    friend std::ostream& operator<<(std::ostream& os,
                                    const InstructionTrace& trace);
};

struct FileTracer : public Tracer {
    virtual ~FileTracer() = default;

    FileTracer(const std::string& filename)
        : itrace{}, out(filename, std::ios::trunc) {}

    void begin(uint64_t pc, uint64_t ir) override {
        itrace.pc = pc;
        itrace.ir = ir;
        itrace.inputs.clear();
        itrace.scalarRegOutput.reset();
        itrace.vectorRegOutput.reset();
        itrace.condcode.reset();
    }

    void end() override {
        out << itrace << "\n\n";
        out.flush();
    }

    void scalarRegInput(CPUState& cpu, const char* name,
                        reg_idx ridx) override {
        itrace.inputs.push_back(
            fmt::format("{}=r{}={:#x}", name, ridx.inner, cpu.r[ridx].raw()));
    }
    void vectorRegInput(CPUState& cpu, const char* name, vreg_idx r) override {}

    void immInput(int64_t imm) override {
        itrace.inputs.push_back(fmt::format("imm={}", imm));
    }

    void writebackScalarReg(CPUState& cpu, const char* name,
                            reg_idx ridx) override {
        itrace.scalarRegOutput = fmt::format("{}={}={:#x}", name, ridx.inner,
                                             cpu.r[ridx], cpu.r[ridx].raw());
    }

    void memWrite(uint64_t addr, u<32> val) override {
        //        itrace.scalarMemWrite = fmt::format("32 : {} = {}", addr,
        //        val.inner);
    }

    void memWrite(uint64_t addr, u<36> val) override {
        //        itrace.scalarMemWrite = fmt::format("36 : {} = {}", addr,
        //        val.inner);
    }

    void memWrite(uint64_t addr, f32x4 val) override {
        //        itrace.vectorMemWrite = fmt::format("M[{}] = {}", addr,
        //        val.inner);
    }

    void memRead32(uint64_t addr, uint32_t val) override {
        //        itrace.scalarMemRead = fmt::format("32 : {} = {}", addr, val);
    }

    void memRead36(uint64_t addr, uint64_t val) override {
        //        itrace.scalarMemRead = fmt::format("36 : {} = {}", addr, val);
    }

    void condcode(condition_t cond) override { itrace.condcode = cond; }

  protected:
    InstructionTrace itrace;
    std::ofstream out;
};
