#pragma once

namespace isa {
enum class ScalarArithmeticOp { Add, Sub, Mul, And, Or, Xor, Shr, Shl };

inline auto scalarArithmeticOpFromArithCode(bits<4> v) -> ScalarArithmeticOp {
    switch (v.inner) {
    case 0b0000: return isa::ScalarArithmeticOp::Add;
    case 0b0001: return isa::ScalarArithmeticOp::Sub;
    case 0b0010: return isa::ScalarArithmeticOp::Mul;
    case 0b0011: return isa::ScalarArithmeticOp::And;
    case 0b0100: return isa::ScalarArithmeticOp::Or;
    case 0b0101: return isa::ScalarArithmeticOp::Xor;
    case 0b0110: return isa::ScalarArithmeticOp::Shr;
    case 0b0111: return isa::ScalarArithmeticOp::Shl;
    default:
        panic("invalid A-format opcode");
    }
}

} // namespace isa

inline std::ostream& operator<<(std::ostream& os, const isa::ScalarArithmeticOp& v) {
    switch (v) {
    case isa::ScalarArithmeticOp::Add: os << "add"; break;
    case isa::ScalarArithmeticOp::Sub: os << "sub"; break;
    case isa::ScalarArithmeticOp::Mul: os << "mul"; break;
    case isa::ScalarArithmeticOp::And: os << "and"; break;
    case isa::ScalarArithmeticOp::Or:  os <<  "or"; break;
    case isa::ScalarArithmeticOp::Xor: os << "xor"; break;
    case isa::ScalarArithmeticOp::Shr: os << "shr"; break;
    case isa::ScalarArithmeticOp::Shl: os << "shl"; break;
    }
    return os;
}