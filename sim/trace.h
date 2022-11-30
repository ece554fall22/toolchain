#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <optional>

struct InstructionTrace {
    uint64_t pc;
    uint64_t ir;

    friend std::ostream &operator<<(std::ostream &os, const InstructionTrace &trace);
};

struct Tracer {
    Tracer(const std::string& filename) : itrace{}, out(filename, std::ios::trunc) {}

    /**
     * Called when entering a fresh instruction.
     * @param pc
     * @param ir
     */
    void begin(uint64_t pc, uint64_t ir) {
        itrace.pc = pc;
        itrace.ir = ir;
    }

    /**
     * Called when the current instruction completes, dumping it to the tracefile ostream.
     */
    void end() {
        assert(out.good());
        out << itrace << "\n\n";
        out.flush();
    }

protected:
    InstructionTrace itrace;
    std::ofstream out;
};
