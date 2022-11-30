#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "morph/encoder.h"

TEST_CASE("emits correct encodings") {
    isa::Emitter emitter;

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

    SUBCASE("subi") {
        emitter.scalarArithmeticImmediate(isa::ScalarArithmeticOp::Sub, /*r*/ 4,
                                          /*r*/ 3, -23);
        CHECK(emitter.getData().back() ==
              0b0010100'00100'00011'111111111101001);
    }

    SUBCASE("mul") {
        emitter.scalarArithmetic(isa::ScalarArithmeticOp::Mul, 3, 4, 1);
        CHECK(emitter.getData().back() ==
              0b0011011'00011'00100'00001'000000'0010);
    }

    // should've only emitted one instruction word
    REQUIRE(emitter.getData().size() == 1);
}
