#pragma once

constexpr int N_CORES = 8;
using coreset_t = bits<N_CORES>;

struct Accelerator {
    void copyToCard(void* hostSrc, uint64_t cardDest, uint64_t len);
    void copyFromCard(void* hostDst, uint64_t cardSrc, uint64_t len);

    void resetCores(coreset_t cores);
    void haltCores(coreset_t cores);
    void unhaltCores(coreset_t cores);
    auto checkDirty() -> coreset_t;
};
