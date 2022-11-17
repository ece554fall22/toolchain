#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>

#include "varint.h"

using addr_t = u<36>;

using reg_idx = u<5>;
using vreg_idx = u<5>;
enum class condition_t {
    nz = 0b000,
    ez = 0b001,
    lz = 0b010,
    gz = 0b011,
    le = 0b100,
    ge = 0b101,
};

inline std::ostream& operator<<(std::ostream& os, const condition_t& v) {
    switch (v) {
    case condition_t::nz:
        os << "nz";
        break;
    case condition_t::ez:
        os << "ez";
        break;
    case condition_t::lz:
        os << "lz";
        break;
    case condition_t::gz:
        os << "gz";
        break;
    case condition_t::le:
        os << "le";
        break;
    case condition_t::ge:
        os << "ge";
        break;
    }
    return os;
}
template <> struct fmt::formatter<condition_t> : ostream_formatter {};

struct f32x4 {
    float v[4];

    f32x4() : f32x4(0.) {}
    f32x4(float a) : f32x4(a, a, a, a) {}
    f32x4(float x, float y, float z, float w) : v{x, y, z, w} {}

    float operator[](size_t idx) const noexcept { return v[idx]; }
    float& operator[](size_t idx) noexcept { return v[idx]; }

    float x() const noexcept { return v[0]; }
    float y() const noexcept { return v[1]; }
    float z() const noexcept { return v[2]; }
    float w() const noexcept { return v[3]; }
};

inline std::ostream& operator<<(std::ostream& os, const f32x4& v) {
    fmt::print(os, "({}, {}, {}, {})", v.x(), v.y(), v.z(), v.w());
    return os;
}
template <> struct fmt::formatter<f32x4> : ostream_formatter {};
