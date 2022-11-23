#include "cpu.h"
#include <morph/ty.h>
#include <morph/varint.h>

// stubs because we don't simulate caches yet
void MemSystem::flushICache() {}
void MemSystem::flushDCacheDirty() {}
void MemSystem::flushDCacheClean() {}
void MemSystem::flushDCacheLine(uint64_t at) {}

void MemSystem::write(uint64_t addr, u<32> val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}

void MemSystem::write(uint64_t addr, u<36> val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}

void MemSystem::write(uint64_t addr, u<64> val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}

void MemSystem::write(uint64_t addr, f32x4 val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}

auto MemSystem::read32(uint64_t addr) -> uint32_t { return 0x32001337; }

auto MemSystem::read36(uint64_t addr) -> uint64_t { return 0x36001337; }
