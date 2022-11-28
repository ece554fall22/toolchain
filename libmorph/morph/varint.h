#pragma once

#include "bit_cast.h"
#include <cassert>
#include <compare>
#include <cstdint>
#include <iostream>
#include <type_traits>

#define BITFILL(n) ((1L << n) - 1)

template <size_t SIZE> struct bits {
    typedef uint64_t inner_t;
    static constexpr size_t backing_size = 64;
    typedef int64_t signed_inner_t;
    static constexpr size_t size = SIZE;
    static constexpr inner_t mask = (1UL << size) - 1;

    static_assert(SIZE <= 64,
                  "size cannot be larger than 64-bit backing storage");
    static_assert(SIZE > 0, "size must be greater than zeros");

    bits() = default;
    bits(inner_t x) {
        assert(x <= mask);
        inner = x;
    }
    inner_t inner;

    bool bit(size_t i) const {
        assert(i >= 0 && i < SIZE);

        return this->inner & (1 << i);
    }

    // extract bits, verilog-style
    template <size_t start, size_t end>
    auto slice() const -> bits<start - end + 1> {
        static_assert(start >= end,
                      "`start` of span must be a more significant bit than the "
                      "`end` of the span");
        static_assert(start < SIZE && end < SIZE,
                      "exceeded the highest possible bit index (SIZE-1)");

        constexpr size_t runlength = start - end + 1;
        inner_t mask = BITFILL(runlength) << end;

        bits<runlength> extracted;
        extracted.inner = (this->inner & mask) >> end;

        return extracted;
    }

    // concat as {this, other}
    template <size_t SIZE_OTHER>
    auto concat(const bits<SIZE_OTHER>& other) const
        -> bits<SIZE + SIZE_OTHER> {
        static_assert(SIZE + SIZE_OTHER <= backing_size,
                      "concatenation of two bitstrings must be smaller than "
                      "the backing storage");
        bits<SIZE + SIZE_OTHER> result;
        result.inner = (this->inner << SIZE_OTHER) | (other.inner);

        return result;
    }

    auto _sgn_inner() const -> signed_inner_t {
        return static_cast<signed_inner_t>(this->inner);
    }

    bits<SIZE>& operator|=(const bits<SIZE>& rhs) {
        this->inner |= rhs.inner;
        return *this;
    }

    bits<SIZE>& operator&=(const bits<SIZE>& rhs) {
        this->inner &= rhs.inner;
        return *this;
    }

    bits<SIZE>& operator^=(const bits<SIZE>& rhs) {
        this->inner ^= rhs.inner;
        return *this;
    }

    friend bits<SIZE> operator&(bits<SIZE> lhs, const bits<SIZE>& rhs) {
        lhs &= rhs;
        return lhs;
    }

    friend bits<SIZE> operator|(bits<SIZE> lhs, const bits<SIZE>& rhs) {
        lhs |= rhs;
        return lhs;
    }

    friend bits<SIZE> operator^(bits<SIZE> lhs, const bits<SIZE>& rhs) {
        lhs ^= rhs;
        return lhs;
    }

    friend bits<SIZE> operator~(bits<SIZE> rhs) {
        rhs.inner = (~rhs.inner) & mask;
        return rhs;
    }

    // we can always check if two bitstrings of same size are identical
    friend auto operator==(const bits<SIZE>& lhs, const bits<SIZE>& rhs) {
        return lhs.inner == rhs.inner;
    }
};

inline auto float2bits(float v) -> bits<32> {
    bits<32> b;
    b.inner = bit_cast<uint32_t>(v);
    return b;
}

inline auto bits2float(bits<32> b) -> float {
    auto bmasked = static_cast<uint32_t>(b.inner & b.mask);
    return bit_cast<float>(bmasked);
}

template <size_t N> struct u;
template <size_t N> struct s;

template <size_t N> struct u : public bits<N> {
    static constexpr typename bits<N>::inner_t max_val = (1UL << N) - 1;

    u() : u(0) {}
    u(uint64_t v) {
        assert(v <= max_val);
        this->inner = static_cast<typename bits<N>::inner_t>(v);
    }
    u(bits<N> b) : bits<N>(b) { assert(b.inner <= max_val); }

    /// Interpret as signed integer data.
    auto asSigned() const noexcept -> s<N> {
        s<N> v;
        v.inner = this->inner;
        return v;
    }

    /// addition of integer constants
    u<N>& operator+=(int rhs) {
        this->inner += rhs;
        return *this;
    }

    operator size_t() const { return this->inner & bits<N>::mask; }

    template <size_t M> u<N>& operator+=(const u<M>& rhs) {
        // todo: assert?
        this->inner += rhs.inner;
        return *this;
    }

    template <size_t M> u<N>& operator+=(const s<M>& rhs) {
        // todo: assert?
        this->inner += static_cast<typename bits<M>::signed_inner_t>(rhs.inner);
        return *this;
    }

    template <size_t M> friend u<N> operator+(u<N> lhs, const u<M>& rhs) {
        lhs += rhs;
        return lhs;
    }

    template <size_t M> friend u<N> operator+(u<N> lhs, const s<M>& rhs) {
        lhs += rhs;
        return lhs;
    }
};

// internally, for ease-of-implementation, we keep a full
// backing-store sized signed integer and operate on that,
// instead of subsetting just N bits. that does mean that
// whenever we convert to/from a N-bit pattern, we need to
// make sure the backing store is signed extended
template <size_t N> struct s : public bits<N> {
    static constexpr typename bits<N>::signed_inner_t max_val =
        (1L << (N - 1)) - 1;
    static constexpr typename bits<N>::signed_inner_t min_val =
        -(1L << (N - 1));

    s() : s(0) {}
    s(int64_t v) {
        assert((min_val <= v) && (v <= max_val));
        this->inner = static_cast<typename bits<N>::inner_t>(v);
    }
    s(bits<N> b) : bits<N>(b) {
        if (this->bit(N - 1)) {
            this->inner |= ((1UL << (bits<N>::backing_size - N)) - 1) << N;
        }

        assert((min_val <= this->_sgn_inner()) &&
               (this->_sgn_inner() <= max_val));
    }

    static auto fromBits(uint64_t x) -> s<N> {
        s<N> v;
        v.inner = x & bits<N>::mask;

        // check sign bit and sign extend
        if (v.bit(N - 1)) {
            v.inner |= ((1UL << (bits<N>::backing_size - N)) - 1) << N;
        }

        return v;
    }

    auto sign() const -> bool { return this->bit(N - 1); }

    auto asUnsigned() const -> u<N> {
        u<N> v;
        v.inner = this->inner;
        return v;
    }

    template <size_t M> s<N>& operator+=(const u<M>& rhs) {
        // todo: assert?
        this->inner = this->_sgn_inner() + rhs.inner;
        return *this;
    }

    template <size_t M> s<N>& operator+=(const s<M>& rhs) {
        // todo: assert?
        this->inner = this->_sgn_inner() + rhs._sgn_inner();
        return *this;
    }

    template <size_t M> s<N>& operator-=(const s<M>& rhs) {
        // todo: assert?
        this->inner = this->_sgn_inner() - rhs._sgn_inner();
        return *this;
    }

    // signed <- signed + unsigned
    template <size_t M> friend s<N> operator+(s<N> lhs, const u<M>& rhs) {
        lhs += rhs;
        return lhs;
    }

    // signed <- signed + signed
    template <size_t M> friend s<N> operator+(s<N> lhs, const s<M>& rhs) {
        lhs += rhs;
        return lhs;
    }

    // signed <- signed - signed
    template <size_t M> friend s<N> operator-(s<N> lhs, const s<M>& rhs) {
        lhs -= rhs;
        return lhs;
    }

    // signed <- signed * signed
    template <size_t M>
    friend s<N + M> operator*(const s<N>& lhs, const s<M>& rhs) {
        // todo: assert?
        s<N + M> v;
        v.inner = lhs._sgn_inner() * rhs._sgn_inner();
        return v;
    }

    // signed <- signed * signed
    // truncating
    template <size_t MSIZE, size_t RSIZE>
    auto truncMult(const s<RSIZE>& rhs) -> s<N> {
        s<MSIZE> v;
        v.inner = this->_sgn_inner() * rhs._sgn_inner();
        v.inner &= bits<MSIZE>::mask;

        return v;
    }

    template <size_t M> auto operator<=>(const s<M>& rhs) const {
        return this->_sgn_inner() <=> rhs._sgn_inner();
    }

    auto operator<=>(int rhs) const { return this->_sgn_inner() <=> rhs; }
    bool operator==(int rhs) const { return this->_sgn_inner() == rhs; }
};

template <size_t N>
inline std::ostream& operator<<(std::ostream& os, const u<N>& v) {
    os << v.inner << "u" << N;
    return os;
}

template <size_t N>
inline std::ostream& operator<<(std::ostream& os, const s<N>& v) {
    os << v._sgn_inner() << "i" << N;
    return os;
}

#undef BITFILL
