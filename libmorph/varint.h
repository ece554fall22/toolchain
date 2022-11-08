#pragma once

#include <cassert>
#include <compare>
#include <cstdint>
#include <iostream>
#include <type_traits>

template <size_t SIZE> struct bits {
    typedef uint64_t inner_t;
    typedef int64_t signed_inner_t;
    static constexpr size_t size = SIZE;
    static constexpr inner_t mask = (1UL << size) - 1;

    static_assert(SIZE <= 64,
                  "size cannot be larger than 64-bit backing storage");
    static_assert(SIZE > 0, "size must be greater than zeros");

    // bits() = default;
    inner_t inner;

    bool bit(size_t i) const {
        assert(i >= 0 && i < SIZE);

        return this->inner & (1 << i);
    }

    auto _sgn_inner() const -> signed_inner_t {
        return static_cast<signed_inner_t>(this->inner);
    }
};

// template<size_t N> using s = varint<true, N>;
// template<size_t N> using u = varint<false, N>;
template <size_t N> struct u;
template <size_t N> struct s;

template <size_t N> struct u : public bits<N> {
    static constexpr typename bits<N>::inner_t max_val = (1UL << N) - 1;

    u() : u(0) {}
    u(uint64_t v) {
        assert(v <= max_val);
        this->inner = static_cast<typename bits<N>::inner_t>(v);
    }

    /// Interpret as signed integer data.
    s<N> asSigned() const {
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
// make sure the backing store is signeds
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

    bool sign() const { return this->bit(N - 1); }

    u<N> asUnsigned() const {
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

    template <size_t M> friend s<N> operator+(s<N> lhs, const u<M>& rhs) {
        lhs += rhs;
        return lhs;
    }

    template <size_t M> friend s<N> operator+(s<N> lhs, const s<M>& rhs) {
        lhs += rhs;
        return lhs;
    }

    template <size_t M> friend s<N> operator-(s<N> lhs, const s<M>& rhs) {
        lhs -= rhs;
        return lhs;
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
