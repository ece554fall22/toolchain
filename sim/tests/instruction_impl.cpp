#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <Eigen/Dense>

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

// Note: The tests here are not up to date with the branch specifically working
// on the vector unit and
//      other tests (ndubuisi/instructions) so be careful merging

TEST_CASE("Matrix Multiply Unit") {
    CPUState cpuState;
    vreg_idx vA;
    vreg_idx vB;
    size_t vA_idx;
    size_t vB_idx;

    SUBCASE("test1") {
        vA_idx = 7;
        vB_idx = 3;
        f32x4 vA = {7.92275497e+29, -6.80173178e+14, -5.23570978e+26,
                    -2.56092934e+14}; // va
        f32x4 vB = {4.34680438e-02, 1.99914452e+38, -8.63642572e+31,
                    2.16317983e+35}; // vb
        // directly inject our test vectors
        cpuState.v[vA_idx] = vA;
        cpuState.v[vB_idx] = vB;

        // Fill up array
        for (int i = 0; i < cpuState.matUnit.MAT_SIZE; i++) {
            instructions::writeA(cpuState, /*vA*/ vA_idx, /*vB*/ vB_idx);
            instructions::writeB(cpuState, /*vA*/ vA_idx, /*vB*/ vB_idx);
        }

        // Initialize
        using Matrix8f = Eigen::Matrix<float, 8, 8>;
        Matrix8f A = Matrix8f::Zero();
        Matrix8f B = Matrix8f::Zero();
        Matrix8f C = Matrix8f::Zero();

        // Compute matrix multiplication
        instructions::matmul(cpuState);

        // Compute to compare
        for (int row = 0; row < cpuState.matUnit.MAT_SIZE; row++) {
            for (size_t i = 0; i < 4; i++) {
                A(row, i) = vA[i];
                B(row, i) = vB[i];
            }
            for (size_t i = 0; i < 4; i++) {
                A(row, i + 4) = vB[i];
                B(row, i + 4) = vB[i];
            }
        }

        // Compute perf test matrix
        C.noalias() += A * B;

        // Check that C is computed correctly
        CHECK(C.isApprox(cpuState.matUnit.C));
    }
}