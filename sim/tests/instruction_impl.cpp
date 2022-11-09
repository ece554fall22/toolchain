#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cpu.h"
#include "instructions.h"

TEST_CASE("test basic arithmetic") {
    CPUState cpuState;
    MemSystem mem;

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
