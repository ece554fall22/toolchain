#include "cpu.h"

// stubs because we don't simulate caches yet
void MemSystem::flushICache() {}
void MemSystem::flushDCacheDirty() {}
void MemSystem::flushDCacheClean() {}
void MemSystem::flushDCacheLine(addr_t at) {}
