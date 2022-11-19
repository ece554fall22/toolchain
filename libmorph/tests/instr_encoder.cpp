#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "morph/encoder.h"

TEST_CASE("emits correct encodings") {
    Emitter emitter;

    SUBCASE("jmp") {
        emitter.jumpPCRel(1337, false);
        CHECK(emitter.getData().back() == 0b0000010'0000000000000010100111001);
    }

    SUBCASE("jal") {
        emitter.jumpPCRel(1338, true);
        CHECK(emitter.getData().back() == 0b0000011'0000000000000010100111010);
    }

    SUBCASE("jmpr") {
        emitter.jumpRegRel(/*r*/ 3, -50, false);
        CHECK(emitter.getData().back() ==
              0b0000100'11111'00011'111111111001110);
    }

    SUBCASE("jalr") {
        emitter.jumpRegRel(/*r*/ 3, -51, true);
        CHECK(emitter.getData().back() ==
              0b0000101'11111'00011'111111111001101);
    }

    // should've only emitted one instruction word
    REQUIRE(emitter.getData().size() == 1);
}
