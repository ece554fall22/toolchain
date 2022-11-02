#pragma once

#include <iostream>
#include <iomanip>
#include <fmt/core.h>

#include "varint.h"

struct Reg {
    u<36> val;
};

std::ostream& operator<<(std::ostream& os, const Reg& reg) {
    os << "reg";
    return os;
}

struct VReg {
    float x;
    float y;
    float z;
    float w;
};


struct ScalarRegisterFile {
    static constexpr size_t N_REGS = 32;
    std::array<Reg, 32> inner;

    ScalarRegisterFile() : inner{} {}

    const Reg& operator[](size_t i) const {
        return inner[i];
    }
};

struct VectorRegisterFile {
    static constexpr size_t N_REGS = 32;
    std::array<VReg, 31> inner;
};

struct ConditionFlags {
    bool zero;
    bool sign;
    bool overflow;
};

struct CPUState {
    ScalarRegisterFile r;
    VectorRegisterFile v;
    ConditionFlags f;

    CPUState() {
    }

    void dump() const {
        std::cout << "scalar registers\n"
                  << "----------------\n";
        for (size_t i = 0; i < ScalarRegisterFile::N_REGS; i++) {
            auto reg = this->r[i];
            std::cout << std::setw(4) << fmt::format("r{}", i) << ": " << reg.val << '\n';
        }

        // std::cout << "vector registers\n"
        //           << "----------------\n";
        // for (size_t i = 0; i < ScalarRegisterFile::N_REGS; i++) {
        //     auto reg = this->v[i];
        //     std::cout << i << ": " << r << '\n';
        // }
    }
};

