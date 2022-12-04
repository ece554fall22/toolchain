#pragma once

#include <pcg_random.hpp>

#include <morph/encoder.h>
#include <morph/isa.h>
#include <morph/ty.h>

using isa::ScalarArithmeticOp;

template <typename RNG, typename T, size_t N>
auto choose(RNG& rng, std::array<T, N>& arr) -> const T& {
    return arr[rng(N)];
}
// template<typename RNG, typename E> auto choose(RNG& rng) -> E {
//     return E::count__;
// }
// template<typename RNG, size_t N> auto rand_bits(RNG& rng) -> bits<N> {
//     return bits<N>(rng(bits<N>::idx))
// }

template <typename RNG> auto randomAOp(RNG& rng) -> ScalarArithmeticOp {
    std::array<ScalarArithmeticOp, 8> aops = {
        ScalarArithmeticOp::Add, ScalarArithmeticOp::Sub,
        ScalarArithmeticOp::Mul, ScalarArithmeticOp::And,
        ScalarArithmeticOp::Or,  ScalarArithmeticOp::Xor,
        ScalarArithmeticOp::Shr, ScalarArithmeticOp::Shl};

    return choose(rng, aops);
}

template <typename RNG> auto randomAIOp(RNG& rng) -> ScalarArithmeticOp {
    std::array<ScalarArithmeticOp, 8> aiops = {
        ScalarArithmeticOp::Add, ScalarArithmeticOp::Sub,
        ScalarArithmeticOp::And, ScalarArithmeticOp::Or,
        ScalarArithmeticOp::Xor, ScalarArithmeticOp::Shr,
        ScalarArithmeticOp::Shl};

    return choose(rng, aiops);
}

template <typename RNG> auto randomRegIdx(RNG& rng) -> reg_idx {
    return reg_idx(rng(reg_idx::mask));
}

template <typename RNG> auto randomVRegIdx(RNG& rng) -> reg_idx {
    return vreg_idx(rng(vreg_idx::mask));
}
