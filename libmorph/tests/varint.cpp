#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "varint.h"

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
