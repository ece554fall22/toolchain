#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>

#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mul, And, Or, Xor, Shr, Shl };
enum class FloatArithmeticOp {Fadd, Fsub, Fmult, Fdiv};
enum class VectorArithmeticOp {Vadd, Vsub, Vmult, Vdiv, Vmax, Vmin};

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

inline auto floatArithmeticOpFromArithCode(bits<3> v) -> FloatArithmeticOp {
    switch (v.inner) {
    case 0b000:
        return isa::FloatArithmeticOp::Fadd;
    case 0b001:
        return isa::FloatArithmeticOp::Fsub;
    case 0b010:
        return isa::FloatArithmeticOp::Fmult;
    case 0b011:
        return isa::FloatArithmeticOp::Fdiv;
    default:
        panic("invalid floating point op");
    }
}

inline auto vectorArithmeticOpFromOpcode(bits<7> v) -> VectorArithmeticOp {
    switch (v.inner) {
    case 0b0011111:
        return isa::VectorArithmeticOp::Vadd;
    case 0b0100000:
        return isa::VectorArithmeticOp::Vsub;
    case 0b0100001:
        return isa::VectorArithmeticOp::Vmult;
    case 0b0100010:
        return isa::VectorArithmeticOp::Vdiv;
    case 0b0110100:
        return isa::VectorArithmeticOp::Vmax;
    case 0b0110101:
        return isa::VectorArithmeticOp::Vmin;
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
