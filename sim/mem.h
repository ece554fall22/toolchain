#pragma once

#include <vector>

#include <morph/ty.h>

struct Tracer;

struct MemSystem {
    explicit MemSystem(size_t size) : MemSystem(size, nullptr) {}
    MemSystem(size_t size, std::shared_ptr<Tracer> tracer)
        : mempool(size, 0), tracer{tracer} {}

    auto size() const -> uint64_t;

    void write(uint64_t addr, u<32> val);
    void write(uint64_t addr, u<36> val);
    void write(uint64_t addr, f32x4 val);

    auto read32(uint64_t addr) -> uint32_t;
    auto read36(uint64_t addr) -> uint64_t;
    auto readVec(uint64_t addr) -> f32x4;

    auto readInstruction(uint64_t addr) -> uint32_t;

    void flushICache();
    void flushDCacheDirty();
    void flushDCacheClean();
    void flushDCacheLine(uint64_t at);

    // private:
    void _check_addr(uint64_t addr, uint32_t alignTo) const;

    std::vector<uint32_t> mempool;
    std::shared_ptr<Tracer> tracer;
};
