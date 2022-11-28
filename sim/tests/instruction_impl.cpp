#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cpu.h"
#include "instructions.h"

TEST_CASE("basic arithmetic") {
    CPUState cpuState;
    MemSystem mem(16);

    SUBCASE("add and addi") {
        // registers start =0
        // r0 <- 1
        // r1 <- 35
        // r3 <- r1 + r0
        instructions::addi(cpuState, mem, /*r*/ 0, /*r*/ 0, 1);
        instructions::addi(cpuState, mem, /*r*/ 1, /*r*/ 1, 35);
        instructions::add(cpuState, mem, /*r*/ 3, /*r*/ 1, /*r*/ 0);

        CHECK(cpuState.r[0] == 1);
        CHECK(cpuState.r[1] == 35);
        CHECK(cpuState.r[3] == 36);
    }
}

TEST_CASE("vector arithmetic") {
    CPUState cpuState;
    MemSystem mem(16);

    SUBCASE("vmul") {
        // directly inject our test vectors
        cpuState.v[1] = {0.3, 0.4, 0.5, 0.6};
        cpuState.v[2] = {0.7, 0.8, 0.9, 1.0};

        // vmul v3, v2, v1
        instructions::vmul(cpuState, mem, /*v*/ 3, /*v*/ 2, /*v*/ 1, 0b1111);

        CHECK(cpuState.v[3][0] == doctest::Approx(0.21));
        CHECK(cpuState.v[3][1] == doctest::Approx(0.32));
        CHECK(cpuState.v[3][2] == doctest::Approx(0.45));
        CHECK(cpuState.v[3][3] == doctest::Approx(0.6));
    }

    SUBCASE("vmul with mask") {
        // directly inject our test vectors
        cpuState.v[1] = {0.3, 0.4, 0.5, 0.6};
        cpuState.v[2] = {0.7, 0.8, 0.9, 1.0};

        // vmul {v3.0, v3.2}, v2, v1
        instructions::vmul(cpuState, mem, /*v*/ 3, /*v*/ 2, /*v*/ 1, 0b0101);

        CHECK(cpuState.v[3][0] == doctest::Approx(0.21));
        CHECK(cpuState.v[3][1] == doctest::Approx(0));
        CHECK(cpuState.v[3][2] == doctest::Approx(0.45));
        CHECK(cpuState.v[3][3] == doctest::Approx(0));
    }
}
