#pragma once

#include <vector>

#include "ty.h"
#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mult, And, Or, Xor, Shr, Shl, Not };
}; // namespace isa

class Emitter {
  public:
    Emitter() : data{} {}

    void jumpPCRel(s<25> imm, bool link);
    void jumpRegRel(reg_idx rA, s<20> imm, bool link);

    void scalarArithmeticImmediate(isa::ScalarArithmeticOp op, reg_idx rD,
                                   reg_idx rA, s<15> imm);
    void scalarArithmetic(isa::ScalarArithmeticOp op, reg_idx rD, reg_idx rA,
                          reg_idx rB);

    auto getData() -> const auto& { return this->data; }

  private:
    void append(uint32_t enc) { data.push_back(enc); }

    std::vector<uint32_t> data;
};
