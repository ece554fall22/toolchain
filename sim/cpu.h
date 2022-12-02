#pragma once

#include <array>
#include <fmt/core.h>
#include <iomanip>
#include <iostream>
#include <vector>

#include <morph/ty.h>
#include <morph/varint.h>

struct MemSystem {
    explicit MemSystem(size_t size) : mempool(size, 0) {}

    void write(uint64_t addr, u<32> val);
    void write(uint64_t addr, u<36> val);
    //    void write(uint64_t addr, u<64> val);
    void write(uint64_t addr, f32x4 val);

    auto read32(uint64_t addr) -> uint32_t;
    auto read36(uint64_t addr) -> uint64_t;

    auto readInstruction(uint64_t addr) -> uint32_t;

    void flushICache();
    void flushDCacheDirty();
    void flushDCacheClean();
    void flushDCacheLine(uint64_t at);

    // private:
    static void _check_alignment(uint64_t addr, uint32_t alignTo);

    std::vector<uint32_t> mempool;
    //    std::shared_ptr<Tracer> tracer;
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
    PC() : current{0}, next{0} {}

    void reset() {
        current = 0;
        next = 0;
    }

    void addToNextPC(int64_t offs) { next += offs; }

    void setNextPC(int64_t pc) { next = pc; }

    auto getNewPC() -> uint64_t {
        current = next;
        next = current + 4;
        return current;
    }

    [[nodiscard]] auto getCurrentPC() const -> uint64_t { return current; }

  private:
    uint64_t current;
    uint64_t next;
};

struct CPUState {
    CPUState() : r{}, v{}, f{}, pc{}, halted{false} {}

    ScalarRegisterFile r;
    VectorRegisterFile v;
    ConditionFlags f;

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
