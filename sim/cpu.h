#pragma once

#include <fmt/core.h>
#include <iomanip>
#include <iostream>
#include <optional>
#include <array>

#include "varint.h"
#include "ty.h"

using reg_idx = u<5>;
using vreg_idx = u<5>;

struct MemSystem {
    void write(addr_t addr, u<36> val);
    void write(addr_t addr, u<64> val);
    void write(addr_t addr, f32x4 val);

    void flushICache();
    void flushDCacheDirty();
    void flushDCacheClean();
    void flushDCacheLine(addr_t at);
};

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
    // const Reg& operator[]()
};

struct ConditionFlags {
    bool zero;
    bool sign;
    bool overflow;
};

std::ostream& operator<<(std::ostream& os, const ConditionFlags& f) {
    if (f.zero)
        os << 'Z';
    if (f.sign)
        os << 'S';
    if (f.overflow)
        os << 'O';

    return os;
}

struct PC {
    addr_t current;
    std::optional<addr_t> redirect_to;

    void redirect(addr_t to) noexcept {
        redirect_to = to;
    }

    /// Return the current PC.
    addr_t get() noexcept {
        return current;
    }

    /// Updates the PC to its next value, returning it.
    /// This is where we decide to either grab PC+4 or the redirect target.
    addr_t update() noexcept {
        if (auto next = std::exchange(redirect_to, std::nullopt)) {
            current = *next;
        } else {
            current += 4;
        }

        return current;
    }
};


struct CPUState {
    ScalarRegisterFile r;
    VectorRegisterFile v;
    ConditionFlags f;

    PC pc;

    CPUState() {}

    void dump() const {
        std::cout << "flags: " << f << "\n\n";
        std::cout << "scalar registers\n"
                  << "----------------\n";
        for (size_t i = 0; i < ScalarRegisterFile::N_REGS; i++) {
            auto reg = this->r[i];
            std::cout << std::setw(4) << fmt::format("r{}", i) << ": "
                      << reg << '\n';
        }

        std::cout << "vector registers\n"
                  << "----------------\n";
        for (size_t i = 0; i < VectorRegisterFile::N_REGS; i++) {
            auto reg = this->v[i];
            std::cout << i << ": " << reg << '\n';
        }
    }
};
