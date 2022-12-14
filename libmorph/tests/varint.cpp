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

TEST_CASE("bits<N> concatenation") {
    auto a = bits<5>(0b10111);
    auto b = bits<7>(0b1110101);

    auto aa = a.concat(a);
    CHECK(aa.inner == 0b10111'10111);
    auto ab = a.concat(b);
    CHECK(ab.inner == 0b10111'1110101);
    auto ba = b.concat(a);
    CHECK(ba.inner == 0b1110101'10111);
    auto bb = b.concat(b);
    CHECK(bb.inner == 0b1110101'1110101);
}

TEST_CASE("bits<N> bitwise manip") {
    auto a = bits<5>(0b10111);
    auto b = bits<5>(0b11101);

    CHECK((~a).inner == 0b01000);
    CHECK((a & b).inner == 0b10101);
    CHECK((a | b).inner == 0b11111);
    CHECK((a ^ b).inner == 0b01010);

    CHECK(a.inner == 0b10111); // check nothing mutated
    CHECK(b.inner == 0b11101); // check nothing mutated
}

TEST_CASE("bits<N> comparison") {
    auto a = bits<5>(0b10111);
    auto b = bits<5>(0b11101);
    auto c = bits<5>(0b10111);

    CHECK(a == c);
    CHECK(c == a);

    CHECK(a != b);
    CHECK(b != a);

    CHECK(b != c);
    CHECK(c != b);
}

TEST_CASE("signed multiplication") {
    auto a = s<5>(4);
    auto b = s<3>(2);
    auto c = s<4>(-2);

    auto ab = a * b;
    auto ba = b * a;
    auto bc = b * c;
    auto cb = c * b;
    auto ca = c * a;
    auto ac = a * c;

    CHECK(ab == s<8>(8));
    // CHECK(ab.size == 8);

    CHECK(bc == s<7>(-4));
    // CHECK(bc.size == 7);

    CHECK(ca == s<9>(-8));
    // CHECK(ca.size == 9);

    // check commutativity
    CHECK(ab == ba);
    CHECK(bc == cb);
    CHECK(ca == ac);
}

TEST_CASE("float2bits and bits2float") {
    float f = 0.45;

    auto bits = float2bits(f);
    float f_roundtrip = bits2float(bits);

    REQUIRE(f == f_roundtrip); // needs to be exact!
}
