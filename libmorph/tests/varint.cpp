#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "morph/varint.h"

TEST_CASE("signed integer extension from bits") {
    auto val = s<4>::fromBits(0b1111);
    CHECK(val._sgn_inner() == -1);
    val = s<4>::fromBits(0b1110);
    CHECK(val._sgn_inner() == -2);

    // shouldn't extend
    val = s<4>::fromBits(0b0000);
    CHECK(val._sgn_inner() == 0);
    val = s<4>::fromBits(0b0001);
    CHECK(val._sgn_inner() == 1);
}

TEST_CASE("bits<N> slicing") {
    //    19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
    // 0b  0  1  1  0  0  0  1  1  0  1 0 0 1 1 1 0 1 0 0 1
    auto val = bits<20>(0b01100011010011101001);

    CHECK(val.slice<0, 0>().inner == 0b1);
    CHECK(val.slice<1, 0>().inner == 0b01);
    CHECK(val.slice<7, 0>().inner == 0b11101001);

    CHECK(val.slice<15, 11>().inner == 0b00110);

    // identity :)
    CHECK(val.slice<19, 0>().inner == 0b01100011010011101001);
}
