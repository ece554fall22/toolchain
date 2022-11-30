#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>

#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mul, And, Or, Xor, Shr, Shl };

inline auto scalarArithmeticOpFromAIOpcode(bits<7> v) -> ScalarArithmeticOp {
    switch (v.inner) {
    case 0b0010011:
        return isa::ScalarArithmeticOp::Add;
    case 0b0010100:
        return isa::ScalarArithmeticOp::Sub;
    case 0b0010101:
        return isa::ScalarArithmeticOp::And;
    case 0b0010110:
        return isa::ScalarArithmeticOp::Or;
    case 0b0010111:
        return isa::ScalarArithmeticOp::Xor;
    case 0b0011000:
        return isa::ScalarArithmeticOp::Shr;
    case 0b0011001:
        return isa::ScalarArithmeticOp::Shl;
    default:
        panic("invalid AI-format opcode");
    }
}

inline auto scalarArithmeticOpFromArithCode(bits<4> v) -> ScalarArithmeticOp {
    switch (v.inner) {
    case 0b0000:
        return isa::ScalarArithmeticOp::Add;
    case 0b0001:
        return isa::ScalarArithmeticOp::Sub;
    case 0b0010:
        return isa::ScalarArithmeticOp::Mul;
    case 0b0011:
        return isa::ScalarArithmeticOp::And;
    case 0b0100:
        return isa::ScalarArithmeticOp::Or;
    case 0b0101:
        return isa::ScalarArithmeticOp::Xor;
    case 0b0110:
        return isa::ScalarArithmeticOp::Shr;
    case 0b0111:
        return isa::ScalarArithmeticOp::Shl;
    default:
        panic("invalid A-format math op");
    }
}

inline std::ostream& operator<<(std::ostream& os,
                                const isa::ScalarArithmeticOp& v) {
    switch (v) {
    case isa::ScalarArithmeticOp::Add:
        os << "add";
        break;
    case isa::ScalarArithmeticOp::Sub:
        os << "sub";
        break;
    case isa::ScalarArithmeticOp::Mul:
        os << "mul";
        break;
    case isa::ScalarArithmeticOp::And:
        os << "and";
        break;
    case isa::ScalarArithmeticOp::Or:
        os << "or";
        break;
    case isa::ScalarArithmeticOp::Xor:
        os << "xor";
        break;
    case isa::ScalarArithmeticOp::Shr:
        os << "shr";
        break;
    case isa::ScalarArithmeticOp::Shl:
        os << "shl";
        break;
    }
    return os;
}

} // namespace isa

template <>
struct fmt::formatter<isa::ScalarArithmeticOp> : ostream_formatter {};
