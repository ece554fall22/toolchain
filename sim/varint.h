#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

template <bool SIGNED, size_t SIZE> class varint {
  public:
    typedef std::conditional<SIGNED, int64_t, uint64_t> ulp_t;
    static constexpr size_t size = SIZE;

    static_assert(SIZE <= 64, "size cannot be larger than 64-bit backing storage");
    static_assert(SIZE > 0, "size must be greater than zeros");

    varint() = default;

    ulp_t ulps() const { return inner; }

  protected:
    static varint<false, SIZE> fromUlps(uint64_t raw) {
        varint<false, SIZE> b;
        b.inner = raw;
    }
    static varint<true, SIZE> fromUlps(int64_t raw) {
        varint<true, SIZE> b;
        b.inner = raw;
    }

    ulp_t inner;
};

// template<size_t N> using s = varint<true, N>;
// template<size_t N> using u = varint<false, N>;
template <size_t N> class u;
template <size_t N> class s;

template <size_t N> class u : public varint<false, N> {
public:
    /// Interpret as signed integer data.
    s<N> asSigned() const {
    }

    bool bit(size_t i) const {
        assert(i >= 0 && i < N);

        return this->inner & (1 << i);
    }
};

// internally, for ease-of-implementation, we keep a full
// backing-store sized signed integer and operate on that,
// instead of subsetting just N bits. that does mean that
// whenever we convert to/from a N-bit pattern, we need to
// make sure the backing store is signeds
template <size_t N> class s : public varint<true, N> {};

template <size_t SIZE>
std::ostream& operator<<(std::ostream& os, const u<SIZE>& v) {
    return os;
}

template <size_t SIZE>
std::ostream& operator<<(std::ostream& os, const s<SIZE>& v) {
    return os;
}


// template <bool SIGNED, size_t SIZE>
// std::ostream& operator<<(std::ostream& os, const varint<SIGNED, SIZE>& v) {
//     os << v.as_u64() << (SIGNED ? "s" : "u") << SIZE;
//     return os;
// }
