#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>

#include "util.h"
#include "varint.h"

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mul, And, Or, Xor, Shr, Shl };
enum class FloatArithmeticOp { Fadd, Fsub, Fmul, Fdiv };
enum class LanewiseVectorOp { Add, Sub, Mul, Div, Min, Max };
enum class VectorScalarOp {
    Add,
    Sub,
    Mul,
    Div,
};
enum class MatrixWriteOp { WriteA, WriteB, WriteC };

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
        return isa::FloatArithmeticOp::Fmul;
    case 0b011:
        return isa::FloatArithmeticOp::Fdiv;
    default:
        panic("invalid floating point op");
    }
}

inline auto vectorArithmeticOpFromOpcode(bits<7> v) -> LanewiseVectorOp {
    switch (v.inner) {
    case 0b0011111:
        return isa::LanewiseVectorOp::Add;
    case 0b0100000:
        return isa::LanewiseVectorOp::Sub;
    case 0b0100001:
        return isa::LanewiseVectorOp::Mul;
    case 0b0100010:
        return isa::LanewiseVectorOp::Div;
    case 0b0110100:
        return isa::LanewiseVectorOp::Max;
    case 0b0110101:
        return isa::LanewiseVectorOp::Min;
    default:
        panic("invalid vector arith op");
    }
}

inline auto vectorScalarArithmeticOpFromOpcode(bits<7> v) -> VectorScalarOp {
    switch (v.inner) {
    case 0b0101001:
        return isa::VectorScalarOp::Add;
    case 0b0101010:
        return isa::VectorScalarOp::Mul;
    case 0b0101011:
        return isa::VectorScalarOp::Sub;
    case 0b0101100:
        return isa::VectorScalarOp::Div;
    default:
        panic("invalid vector scalar arith op");
    }
}

inline auto matrixWriteOpFromOpcode(bits<7> v) -> MatrixWriteOp {
    switch (v.inner) {
    case 0b0101100:
        return isa::MatrixWriteOp::WriteA;
    case 0b0101111:
        return isa::MatrixWriteOp::WriteB;
    case 0b0110000:
        return isa::MatrixWriteOp::WriteC;
    default:
        panic("invalid matrix write op");
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

inline std::ostream& operator<<(std::ostream& os,
                                const isa::FloatArithmeticOp& v) {
    switch (v) {
    case FloatArithmeticOp::Fadd:
        os << "fadd";
        break;
    case FloatArithmeticOp::Fsub:
        os << "fsub";
        break;
    case FloatArithmeticOp::Fmul:
        os << "fmult";
        break;
    case FloatArithmeticOp::Fdiv:
        os << "fdiv";
        break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const isa::MatrixWriteOp& v) {
    switch (v) {
    case MatrixWriteOp::WriteA:
        os << "writeA";
        break;
    case MatrixWriteOp::WriteB:
        os << "writeB";
        break;
    case MatrixWriteOp::WriteC:
        os << "writeC";
        break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const isa::LanewiseVectorOp& v) {
    switch (v) {
    case LanewiseVectorOp::Add:
        os << "add";
        break;
    case LanewiseVectorOp::Sub:
        os << "sub";
        break;
    case LanewiseVectorOp::Mul:
        os << "mul";
        break;
    case LanewiseVectorOp::Div:
        os << "div";
        break;
    case LanewiseVectorOp::Max:
        os << "max";
        break;
    case LanewiseVectorOp::Min:
        os << "min";
        break;
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os,
                                const isa::VectorScalarOp& v) {
    switch (v) {
    case VectorScalarOp::Add:
        os << "add";
        break;
    case VectorScalarOp::Sub:
        os << "sub";
        break;
    case VectorScalarOp::Mul:
        os << "mul";
        break;
    case VectorScalarOp::Div:
        os << "div";
        break;
    }
    return os;
}

} // namespace isa

template <>
struct fmt::formatter<isa::ScalarArithmeticOp> : ostream_formatter {};

template <>
struct fmt::formatter<isa::FloatArithmeticOp> : ostream_formatter {};

template <> struct fmt::formatter<isa::VectorScalarOp> : ostream_formatter {};

template <> struct fmt::formatter<isa::LanewiseVectorOp> : ostream_formatter {};

template <> struct fmt::formatter<isa::MatrixWriteOp> : ostream_formatter {};
