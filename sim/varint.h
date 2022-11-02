#pragma once

#include <cassert>
#include <cstdint>

template <bool SIGNED, size_t SIZE> class varint {
  public:
    static constexpr bool is_signed = SIGNED;
    static constexpr size_t size = SIZE;

    static_assert(SIZE <= 64, "size cannot be larger than u64 backing storage");
    static_assert(SIZE > 0, "size must be greater than zeros");

    varint() = default;

    uint64_t as_u64() const { return inner; }

    bool bit(size_t i) const {
        assert(i >= 0 && i < SIZE);

        return inner & (1 << i);
    }

  private:
    uint64_t inner;
};

// template<size_t N> using s = varint<true, N>;
// template<size_t N> using u = varint<false, N>;
template <size_t N> class u;
template <size_t N> class s;

template <size_t N> class u : public varint<false, N> {
    template <size_t M> s<M> asSigned() {
        // sign ext
    }
};

template <size_t N> class s : public varint<true, N> {};

template <bool SIGNED, size_t SIZE>
std::ostream& operator<<(std::ostream& os, const varint<SIGNED, SIZE>& v) {
    os << v.as_u64() << (SIGNED ? "s" : "u") << SIZE;
    return os;
}
