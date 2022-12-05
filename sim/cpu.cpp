#include "cpu.h"
#include <morph/ty.h>
#include <morph/util.h>
#include <morph/varint.h>

// stubs because we don't simulate caches yet
void MemSystem::flushICache() {}
void MemSystem::flushDCacheDirty() {}
void MemSystem::flushDCacheClean() {}
void MemSystem::flushDCacheLine(uint64_t at) {}

void MemSystem::write(uint64_t addr, u<32> val) {
    //    tracer->memWrite(addr, val);
    std::cout << "M[" << addr << "]/32 = " << val << "\n";
}

void MemSystem::write(uint64_t addr, u<36> val) {
    //    tracer->memWrite(addr, val);
    std::cout << "M[" << addr << "]/36 = " << val << "\n";
}

// void MemSystem::write(uint64_t addr, u<64> val) {
//     tracer->memWrite(addr, val);
//     std::cout << "M[" << addr << "] = " << val << "\n";
// }

void MemSystem::write(uint64_t addr, f32x4 val) {
    //    tracer->memWrite(addr, val);
    std::cout << "M[" << addr << "] = " << val << "\n";
}

auto MemSystem::read32(uint64_t addr) -> uint32_t {
    _check_addr(addr, 32);

    auto val = this->mempool[addr / 4];
    //    tracer->memRead32(addr, val);
    return val;
}

auto MemSystem::read36(uint64_t addr) -> uint64_t {
    _check_addr(addr, 64);

    // little-endian
    uint64_t val = this->mempool[addr / 4]; // lower
    val |= static_cast<uint64_t>(this->mempool[1 + addr / 4]) << 32;

    //    tracer->memRead36(addr, val);

    return val;
}

auto MemSystem::readInstruction(uint64_t addr) -> uint32_t {
    _check_addr(addr, 32);

    // /4 to get u32-addr instead of byte addr
    return this->mempool[addr / 4];
}

void MemSystem::_check_addr(uint64_t addr, uint32_t alignTo) const {
    // alignTo in bits
    if ((addr % (alignTo / 8)) != 0)
        panic("misaligned memory address");

    if (addr > mempool.size())
        panic("access past end of emulated memory");
}
