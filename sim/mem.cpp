#include "mem.h"
#include <morph/bit_cast.h>
#include <morph/ty.h>
#include <morph/util.h>
#include <morph/varint.h>

#include "trace.h"

// stubs because we don't simulate caches yet
void MemSystem::flushICache() {}
void MemSystem::flushDCacheDirty() {}
void MemSystem::flushDCacheClean() {}
void MemSystem::flushDCacheLine(uint64_t at) {}

void MemSystem::write(uint64_t addr, u<32> val) {
    _check_addr(addr, 32);
    tracer->memWrite(addr, val);

    this->mempool[addr / 4] = val.raw();
}

void MemSystem::write(uint64_t addr, u<36> val) {
    _check_addr(addr, 64);
    tracer->memWrite(addr, val);

    this->mempool[0 + addr / 4] = val.slice<31, 0>().raw();
    this->mempool[1 + addr / 4] = val.slice<35, 32>().raw();
}

void MemSystem::write(uint64_t addr, f32x4 val) {
    _check_addr(addr, 128);
    tracer->memWrite(addr, val);

    size_t base = addr / 4;
    this->mempool[base + 0] = bit_cast<uint32_t>(val.x());
    this->mempool[base + 1] = bit_cast<uint32_t>(val.y());
    this->mempool[base + 2] = bit_cast<uint32_t>(val.z());
    this->mempool[base + 3] = bit_cast<uint32_t>(val.w());
}

auto MemSystem::read32(uint64_t addr) -> uint32_t {
    _check_addr(addr, 32);

    auto val = this->mempool[addr / 4];

    tracer->memRead32(addr, val);
    return val;
}

auto MemSystem::read36(uint64_t addr) -> uint64_t {
    _check_addr(addr, 64);

    // little-endian
    uint64_t val = this->mempool[addr / 4]; // lower
    val |= static_cast<uint64_t>(this->mempool[1 + addr / 4]) << 32;

    tracer->memRead36(addr, val);
    return val;
}

auto MemSystem::readVec(uint64_t addr) -> f32x4 {
    _check_addr(addr, 128);

    size_t base = addr / 4;
    f32x4 vec{
        bit_cast<float>(this->mempool[base + 0]),
        bit_cast<float>(this->mempool[base + 1]),
        bit_cast<float>(this->mempool[base + 2]),
        bit_cast<float>(this->mempool[base + 3]),
    };

    tracer->memReadVec(addr, vec);
    return vec;
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
