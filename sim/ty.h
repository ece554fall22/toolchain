#pragma once

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <iostream>

using addr_t = u<36>;

struct f32x4 {
    float v[4];

    f32x4() : f32x4(0.) {}
    f32x4(float a) : f32x4(a, a, a, a) {}
    f32x4(float x, float y, float z, float w) : v{x,y,z,w} {}

    float operator[](size_t idx) noexcept {
        return v[idx];
    }

    float x() const noexcept { return v[0]; }
    float y() const noexcept { return v[1]; }
    float z() const noexcept { return v[2]; }
    float w() const noexcept { return v[3]; }
};

std::ostream& operator<<(std::ostream& os, const f32x4& v) {
    fmt::print(os, "({}, {}, {}, {})", v.x(), v.y(), v.z(), v.w());
    return os;
}
