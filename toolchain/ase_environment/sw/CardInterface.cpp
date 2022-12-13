
#include "CardInterface.h"

CardInterface::CardInterface(AFU&& afu, uint8_t* cardMem, uint64_t totalSize)
    : afu{std::move(afu)} {
    cardMem = afu.malloc<dma_data_t>((size_t)totalSize); // not sure if this works lol
}

CardInterface::~CardInterface() { afu.free(cardMem); }

void CardInterface::copyToCard(void* hostSrc, uint64_t cardDest, uint64_t len) {
    for (int i = 0; i < len; i++) {
        cardMem[cardDest + i] = hostSrc[cardDest + i];
    }
}

void CardInterface::copyFromCard(void* hostDst, uint64_t cardSrc,
                                 uint64_t len) {
    for (int i = 0; i < len; i++) {
        hostDst[cardSrc + i] = cardMem[cardSrc + i];
    }

    void CardInterface::sendStart() { // may or may not keep this one
        afu.write(MMIO_START_ADDR, (uint64_t)cardMem);
    }

    void CardInterface::resetCores(
        uint64_t cores) { // bc we are only synthing 1 core, I won't use this
                          // argument here
        afu.write(MMIO_RESET, 1);
    }

    void CardInterface::haltCores(
        uint64_t cores) { // bc we are only synthing 1 core, I won't use this
                          // argument here
        afu.write(MMIO_UNHALT, 0);
    }

    void CardInterface::unhaltCores(
        uint64_t cores) { // bc we are only synthing 1 core, I won't use this
                          // argument here
        afu.write(MMIO_UNHALT, 1);
    }

    auto CardInterface::checkDirty()->uint64_t { return afu.read(MMIO_DONE); }
