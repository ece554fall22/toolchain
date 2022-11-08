#include "cpu.h"
#include <ty.h>
#include <varint.h>

// stubs because we don't simulate caches yet
void MemSystem::flushICache() {}
void MemSystem::flushDCacheDirty() {}
void MemSystem::flushDCacheClean() {}
void MemSystem::flushDCacheLine(addr_t at) {}

void MemSystem::write(addr_t addr, u<36> val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}

void MemSystem::write(addr_t addr, u<64> val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}

void MemSystem::write(addr_t addr, f32x4 val) {
    std::cout << "M[" << addr << "] = " << val << "\n";
}
