#pragma once

#include "AFU.h"

class CardInterface {
  public:
    AFU afu;
    uint8_t* cardMem;
    CardInterface(AFU&& afu, uint8_t* cardMem);

    virtual ~CardInterface();

    virtual void copyToCard(void* hostSrc, uint64_t cardDest, uint64_t len);

    virtual void copyFromCard(void* hostDst, uint64_t cardSrc, uint64_t len);

    virtual void sendStart();

    virtual void resetCores(uint64_t cores);

    virtual void haltCores(uint64_t cores);

    virtual void unhaltCores(uint64_t cores);

    virtual auto checkDirty() -> uint64_t;
};
