#pragma once

#include <Eigen/Dense>
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
    static constexpr size_t MAT_SIZE = 8;
    using Matrix = Eigen::Matrix<float, MAT_SIZE, MAT_SIZE>;
    static constexpr size_t SYSTOLIC_CYCLES = 24;

    MatrixUnit()
        : A{Matrix::Zero()}, B{Matrix::Zero()}, C{Matrix::Zero()},
          systolicCycleCt{0} {}

    Matrix A;
    Matrix B;
    Matrix C;

    size_t systolicCycleCt;
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
