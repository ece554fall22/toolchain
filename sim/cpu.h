#pragma once

#include <array>
#include <fmt/core.h>
#include <iomanip>
#include <iostream>
#include <vector>

#include <morph/ty.h>
#include <morph/varint.h>

#include "mem.h"

struct ScalarRegisterFile {
    using Reg = u<36>;
    static constexpr size_t N_REGS = 32;

    std::array<Reg, N_REGS> inner;

    ScalarRegisterFile() : inner{} {}

    const Reg& operator[](size_t i) const { return inner[i]; }
    Reg& operator[](size_t i) { return inner[i]; }
};

// TODO: v0 = (0,0,0,0)
struct VectorRegisterFile {
    using Reg = f32x4;
    static constexpr size_t N_REGS = 32;

    std::array<Reg, N_REGS> inner;

    VectorRegisterFile() : inner{} {}

    const Reg& operator[](size_t i) const { return inner[i]; }
    Reg& operator[](size_t i) { return inner[i]; }
};

struct ConditionFlags {
    bool zero = false;
    bool sign = false;
    bool overflow = false;
};

inline std::ostream& operator<<(std::ostream& os, const ConditionFlags& f) {
    if (f.zero)
        os << 'Z';
    if (f.sign)
        os << 'S';
    if (f.overflow)
        os << 'O';

    return os;
}

struct PC {
    PC() : current{0}, taken{false}, aNotTaken{0}, aTaken{0} {}

    void reset() {
        current = 0;
        aNotTaken = 0;
        aTaken = 0;
        taken = false;
    }

    void addToTakenPC(int64_t offs) { aTaken += offs; }
    void setTakenPC(int64_t a) { aTaken = a; }
    void setTaken(bool v) { taken = v; }

    auto getNewPC() -> uint64_t {
        current = taken ? aTaken : aNotTaken;
        aNotTaken = current + 4;
        aTaken = aNotTaken; // unless jumping/branching :)
        taken = false;

        return current;
    }

    [[nodiscard]] auto getCurrentPC() const -> uint64_t { return current; }

    [[nodiscard]] auto peekNotTaken() const -> uint64_t { return aNotTaken; }
    [[nodiscard]] auto peekTaken() const -> uint64_t { return aTaken; }
    [[nodiscard]] auto wasTaken() const -> uint64_t { return taken; }

  private:
    uint64_t current;

    bool taken;

    uint64_t aNotTaken;
    uint64_t aTaken;
};

struct MatrixUnit {
    using Matrix8f = Eigen::Matrix<float, 8, 8>;
    static constexpr size_t MAT_SIZE = 8;
    
    Matrix8f A = Matrix8f::Zero();
    Matrix8f B = Matrix8f::Zero();
    Matrix8f C = Matrix8f::Zero();

    size_t A_pos; // Keep track of the postion of A during a write
    size_t B_pos;
    size_t C_pos;

    MatrixUnit() : A{} {}
    MatrixUnit() : B{} {}
    MatrixUnit() : C{} {}

    // Increment postion for next write
    void incrementAPos(){ A_pos = (A_pos + 1) % MAT_SIZE; }
    void incrementBPos(){ B_pos = (B_pos + 1) % MAT_SIZE; }
    void incrementCPos(){ C_pos = (C_pos + 1) % MAT_SIZE; }

    // Get current position
    // Increment postion for next write
    size_t getAPos(){ return A_pos;}
    size_t getBPos(){ return B_pos; }
    size_t getCPos(){ return C_pos; }
};

struct CPUState {
    CPUState() : r{}, v{}, f{}, pc{}, halted{false} {}

    ScalarRegisterFile r;
    VectorRegisterFile v;
    ConditionFlags f;
    MatrixUnit matUnit;

    PC pc;

    bool halted;

    [[nodiscard]] auto isHalted() const -> bool { return halted; }
    void halt() { halted = true; }

    void dump() const {
        std::cout << "pc: " << pc.getCurrentPC() << '\n';
        std::cout << "flags: " << f << "\n\n";
        std::cout << "scalar registers\n"
                  << "----------------\n";
        for (size_t i = 0; i < ScalarRegisterFile::N_REGS; i++) {
            auto reg = this->r[i];
            std::cout << std::setw(18)
                      << fmt::format("r{}: {:#011x}\n", i, reg.inner);
        }

        std::cout << "vector registers\n"
                  << "----------------\n";
        for (size_t i = 0; i < VectorRegisterFile::N_REGS; i++) {
            auto reg = this->v[i];
            std::cout << std::setw(4) << fmt::format("v{}", i) << ": " << reg
                      << '\n';
        }
    }
};
