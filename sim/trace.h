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

    virtual void immInput(int64_t imm) = 0;
    virtual void vectorMask(vmask_t mask) = 0;
    virtual void branchCondcode(condition_t cond) = 0;
    virtual void swizzleInput(vlaneidx_t i0, vlaneidx_t i1, vlaneidx_t i2,
                              vlaneidx_t i3) = 0;

    virtual void scalarRegInput(CPUState& cpu, const char* name, reg_idx r) = 0;
    virtual void vectorRegInput(CPUState& cpu, const char* name,
                                vreg_idx r) = 0;

    virtual void scalarRegOutput(CPUState& cpu, const char* name,
                                 reg_idx r) = 0;
    virtual void vectorRegOutput(CPUState& cpu, const char* name,
                                 vreg_idx r) = 0;

    virtual void flagsWriteback(ConditionFlags flags) = 0;
    virtual void controlFlow(PC& pc) = 0;

    virtual void memWrite(uint64_t addr, u<32> val) = 0;
    virtual void memWrite(uint64_t addr, u<36> val) = 0;
    virtual void memWrite(uint64_t addr, f32x4 val) = 0;
    virtual void memRead32(uint64_t addr, uint32_t val) = 0;
    virtual void memRead36(uint64_t addr, uint64_t val) = 0;
    virtual void memReadVec(uint64_t addr, f32x4 val) = 0;
};

struct NullTracer : public Tracer {
    virtual ~NullTracer() = default;

    void begin(uint64_t pc, uint64_t ir) override {}

    void end() override {}

    void vectorMask(vmask_t mask) override {}
    void immInput(int64_t imm) override {}
    void branchCondcode(condition_t cond) override {}
    void swizzleInput(vlaneidx_t i0, vlaneidx_t i1, vlaneidx_t i2,
                      vlaneidx_t i3) override {}

    void scalarRegInput(CPUState& cpu, const char* name, reg_idx r) override {}
    void vectorRegInput(CPUState& cpu, const char* name, vreg_idx r) override {}

    void scalarRegOutput(CPUState& cpu, const char* name, reg_idx r) override {}
    void vectorRegOutput(CPUState& cpu, const char* name, vreg_idx r) override {
    }

    void flagsWriteback(ConditionFlags flags) override {}
    void controlFlow(PC& pc) override {}

    void memWrite(uint64_t addr, u<32> val) override {}
    void memWrite(uint64_t addr, u<36> val) override {}
    void memWrite(uint64_t addr, f32x4 val) override {}
    void memRead32(uint64_t addr, uint32_t val) override {}
    void memRead36(uint64_t addr, uint64_t val) override {}
    void memReadVec(uint64_t addr, f32x4 val) override {}
};

struct InstructionTrace {
    uint64_t pc;
    uint64_t ir;
    std::vector<std::string> inputs;
    std::optional<condition_t> condcode;
    std::optional<std::string> scalarRegOutput;
    std::optional<std::string> vectorRegOutput;
    std::optional<vmask_t> vectorMask;
    std::optional<condition_t> branchConditionCode;
    std::optional<ConditionFlags> flagsWriteback;
    std::optional<std::string> controlFlow;

    std::optional<std::string> scalarLoad, scalarStore, vectorLoad, vectorStore;

    friend std::ostream& operator<<(std::ostream& os,
                                    const InstructionTrace& trace);
};

struct FileTracer : public Tracer {
    virtual ~FileTracer() = default;

    explicit FileTracer(const std::string& filename)
        : itrace{}, out(filename, std::ios::trunc) {}

    void begin(uint64_t pc, uint64_t ir) override {
        itrace.pc = pc;
        itrace.ir = ir;
        itrace.inputs.clear();
        itrace.scalarRegOutput.reset();
        itrace.vectorRegOutput.reset();
        itrace.condcode.reset();
        itrace.vectorMask.reset();
        itrace.branchConditionCode.reset();
        itrace.flagsWriteback.reset();
        itrace.controlFlow.reset();

        itrace.scalarLoad.reset();
        itrace.scalarStore.reset();
        itrace.vectorLoad.reset();
        itrace.vectorStore.reset();
    }

    void end() override {
        out << itrace << "\n\n";
        out.flush();
    }

    // -- const inputs
    void immInput(int64_t imm) override {
        itrace.inputs.push_back(fmt::format("imm={}", imm));
    }
    void vectorMask(vmask_t mask) override { itrace.vectorMask = mask; }
    void branchCondcode(condition_t cond) override { itrace.condcode = cond; }
    void swizzleInput(vlaneidx_t i0, vlaneidx_t i1, vlaneidx_t i2,
                      vlaneidx_t i3) override {
        itrace.inputs.push_back(fmt::format("i0={} i1={} i2={} i3={}", i0.raw(),
                                            i1.raw(), i2.raw(), i3.raw()));
    }

    // -- reg inputs
    void scalarRegInput(CPUState& cpu, const char* name,
                        reg_idx ridx) override {
        itrace.inputs.push_back(
            fmt::format("{}=r{}={:#x}", name, ridx.inner, cpu.r[ridx].raw()));
    }
    void vectorRegInput(CPUState& cpu, const char* name,
                        vreg_idx vidx) override {
        itrace.inputs.push_back(
            fmt::format("{}=v{}={}", name, vidx.inner, cpu.v[vidx]));
    }

    // -- writeback
    void scalarRegOutput(CPUState& cpu, const char* name,
                         reg_idx ridx) override {
        itrace.scalarRegOutput = fmt::format("{}=r{}={:#x}", name, ridx.inner,
                                             cpu.r[ridx], cpu.r[ridx].raw());
    }
    void vectorRegOutput(CPUState& cpu, const char* name,
                         vreg_idx vidx) override {
        itrace.vectorRegOutput = fmt::format("{}=v{}={}", name, vidx.inner,
                                             cpu.v[vidx], cpu.v[vidx]);
    }

    void flagsWriteback(ConditionFlags flags) override {
        itrace.flagsWriteback = flags;
    }
    void controlFlow(PC& pc) override {
        itrace.controlFlow = fmt::format(
            "taken={} taken_addr={:#x} not_taken_addr={:#x}",
            pc.wasTaken() ? 1 : 0, pc.peekTaken(), pc.peekNotTaken());
    }

    // -- memory transactions
    void memWrite(uint64_t addr, u<32> val) override {
        itrace.scalarStore = fmt::format("32 : {:#x} = {:#x}", addr, val.raw());
    }

    void memWrite(uint64_t addr, u<36> val) override {
        itrace.scalarStore = fmt::format("36 : {:#x} = {:#x}", addr, val.raw());
    }

    void memWrite(uint64_t addr, f32x4 val) override {
        itrace.vectorStore = fmt::format("{:#x} = {}", addr, val);
    }

    void memRead32(uint64_t addr, uint32_t val) override {
        itrace.scalarLoad = fmt::format("32 : {:#x} = {:#x}", addr, val);
    }

    void memRead36(uint64_t addr, uint64_t val) override {
        itrace.scalarLoad = fmt::format("36 : {:#x} = {:#x}", addr, val);
    }

    void memReadVec(uint64_t addr, f32x4 val) override {
        itrace.vectorLoad = fmt::format("{:#x} = {}", addr, val);
    }

  protected:
    InstructionTrace itrace;
    std::ofstream out;
};
