#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cpu.h"
#include "instructions.h"
#include <iostream>
using namespace std;

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
    // Providing a seed value
	srand((unsigned) time(NULL));
    CPUState cpuState;
    MemSystem mem(16);
    int num_mask_variations = 4;
    size_t N_REGS = 32;

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

    // vadd
    SUBCASE("vadd") {
        // directly inject our test vectors
        cpuState.v[4] = {7.92275497e+29, -6.80173178e+14, -5.23570978e+26, -2.56092934e+14}; // va
        cpuState.v[2] = {4.34680438e-02,  1.99914452e+38, -8.63642572e+31,  2.16317983e+35}; // vb
        f32x4 vD = {7.92275497e+29,  1.99914452e+38, -8.63647808e+31,  2.16317983e+35};   // result

        // vadd v9, v4, v2
        instructions::vadd(cpuState, mem, /*v*/ 9, /*v*/ 4, /*v*/ 2, 0b1111);

        CHECK(cpuState.v[9][0] == doctest::Approx(vD[0]));
        CHECK(cpuState.v[9][1] == doctest::Approx(vD[1]));
        CHECK(cpuState.v[9][2] == doctest::Approx(vD[2]));
        CHECK(cpuState.v[9][3] == doctest::Approx(vD[3]));
    }

    // Idea: Ensure most/all 32 registers work and works with different masks
    for(int i = 0; i < num_mask_variations; i++){
        vreg_idx vD_idx = rand() % N_REGS;
        vreg_idx vA_idx = rand() % N_REGS;
        vreg_idx vB_idx = rand() % N_REGS;
        instructions::vmask_t mask = 0; 

        // Ensure vA and vB are different
        while(vA_idx._sgn_inner() == vB_idx._sgn_inner()){
            vA_idx = rand() % N_REGS;
            vB_idx = rand() % N_REGS;
        }  

        // already tested mask 0b1111
        while((mask = rand() % 15)._sgn_inner() == 0b1111);

        cpuState.v[vD_idx] = f32x4(); // zero out destination register
        cpuState.v[vA_idx] = {7.92275497e+29, -6.80173178e+14, -5.23570978e+26, -2.56092934e+14}; // va
        cpuState.v[vB_idx] = {4.34680438e-02,  1.99914452e+38, -8.63642572e+31,  2.16317983e+35}; // vb
        f32x4 vD = {7.92275497e+29,  1.99914452e+38, -8.63647808e+31,  2.16317983e+35};   // result
        f32x4 vD_before = cpuState.v[vD_idx]; // vD_idx could be equal to vA_idx or vB_idx

        SUBCASE("vadd with mask") {
            // vadd
            instructions::vadd(cpuState, mem, /*v*/ vD_idx, /*v*/ vA_idx, /*v*/ vB_idx, mask);
            CHECK(cpuState.v[vD_idx][0] == doctest::Approx(mask.bit(0) ? vD[0] : vD_before[0]));
            CHECK(cpuState.v[vD_idx][1] == doctest::Approx(mask.bit(1) ? vD[1] : vD_before[1]));
            CHECK(cpuState.v[vD_idx][2] == doctest::Approx(mask.bit(2) ? vD[2] : vD_before[2]));
            CHECK(cpuState.v[vD_idx][3] == doctest::Approx(mask.bit(3) ? vD[3] : vD_before[3]));
        }
    }

    // vsub
    SUBCASE("vsub") {
        // directly inject our test vectors
        cpuState.v[4] = {-6.74588872e+21,  4.62804803e+10, -3.89988558e+37,  1.19135168e+11}; // va
        cpuState.v[2] = {-1.39457307e+18, -1.39266825e+20, -1.55607182e-03,  1.30704492e+20}; // vb
        f32x4 vD = {-6.74449415e+21,  1.39266825e+20, -3.89988558e+37, -1.30704492e+20};   // result

        // vadd v9, v4, v2
        instructions::vsub(cpuState, mem, /*v*/ 9, /*v*/ 4, /*v*/ 2, 0b1111);

        CHECK(cpuState.v[9][0] == doctest::Approx(vD[0]));
        CHECK(cpuState.v[9][1] == doctest::Approx(vD[1]));
        CHECK(cpuState.v[9][2] == doctest::Approx(vD[2]));
        CHECK(cpuState.v[9][3] == doctest::Approx(vD[3]));
    }

    for(int i = 0; i < num_mask_variations; i++){
        vreg_idx vD_idx = rand() % N_REGS;
        vreg_idx vA_idx = rand() % N_REGS;
        vreg_idx vB_idx = rand() % N_REGS;
        instructions::vmask_t mask = 0; 

        // Ensure vA and vB are different
        while(vA_idx._sgn_inner() == vB_idx._sgn_inner()){
            vA_idx = rand() % N_REGS;
            vB_idx = rand() % N_REGS;
        }      

        // already tested mask 0b1111
        while((mask = rand() % 15)._sgn_inner() == 0b1111);

        cpuState.v[vD_idx] = f32x4(); // zero out destination register
        cpuState.v[vA_idx] = {-6.74588872e+21,  4.62804803e+10, -3.89988558e+37,  1.19135168e+11}; // va
        cpuState.v[vB_idx] = {-1.39457307e+18, -1.39266825e+20, -1.55607182e-03,  1.30704492e+20}; // vb
        f32x4 vD = {-6.74449415e+21,  1.39266825e+20, -3.89988558e+37, -1.30704492e+20};   // result
        f32x4 vD_before = cpuState.v[vD_idx]; // vD_idx could be equal to vA_idx or vB_idx

        SUBCASE("vsub with mask") {
            // vsub
            instructions::vsub(cpuState, mem, /*v*/ vD_idx, /*v*/ vA_idx, /*v*/ vB_idx, mask);
            CHECK(cpuState.v[vD_idx][0] == doctest::Approx(mask.bit(0) ? vD[0] : vD_before[0]));
            CHECK(cpuState.v[vD_idx][1] == doctest::Approx(mask.bit(1) ? vD[1] : vD_before[1]));
            CHECK(cpuState.v[vD_idx][2] == doctest::Approx(mask.bit(2) ? vD[2] : vD_before[2]));
            CHECK(cpuState.v[vD_idx][3] == doctest::Approx(mask.bit(3) ? vD[3] : vD_before[3]));
        }
    }

     // vdiv
    SUBCASE("vdiv") {
        // directly inject our test vectors
        cpuState.v[4] = {-1.98036415e+27, -1.89747182e+10,  4.73326294e-05, -7.29880513e+36}; // va
        cpuState.v[2] = {-3.52366314e+35, -8.52232614e+28, -1.54292809e+23,  1.49007288e+03}; // vb
        f32x4 vD = {5.62018577e-09,  2.22647172e-19, -3.06771454e-28, -4.89828736e+33};   // result

        // vdiv v9, v4, v2
        instructions::vdiv(cpuState, mem, /*v*/ 9, /*v*/ 4, /*v*/ 2, 0b1111);

        CHECK(cpuState.v[9][0] == doctest::Approx(vD[0]));
        CHECK(cpuState.v[9][1] == doctest::Approx(vD[1]));
        CHECK(cpuState.v[9][2] == doctest::Approx(vD[2]));
        CHECK(cpuState.v[9][3] == doctest::Approx(vD[3]));
    }

    for(int i = 0; i < num_mask_variations; i++){
        vreg_idx vD_idx = rand() % N_REGS;
        vreg_idx vA_idx = rand() % N_REGS;
        vreg_idx vB_idx = rand() % N_REGS;
        instructions::vmask_t mask = 0; 

        // Ensure vA and vB are different
        while(vA_idx._sgn_inner() == vB_idx._sgn_inner()){
            vA_idx = rand() % N_REGS;
            vB_idx = rand() % N_REGS;
        }

        // already tested mask 0b1111
        while((mask = rand() % 15)._sgn_inner() == 0b1111);

        cpuState.v[vD_idx] = f32x4(); // zero out destination register
        cpuState.v[vA_idx] = {-1.98036415e+27, -1.89747182e+10,  4.73326294e-05, -7.29880513e+36}; // va
        cpuState.v[vB_idx] = {-3.52366314e+35, -8.52232614e+28, -1.54292809e+23,  1.49007288e+03}; // vb
        f32x4 vD = {5.62018577e-09,  2.22647172e-19, -3.06771454e-28, -4.89828736e+33};   // result
        f32x4 vD_before = cpuState.v[vD_idx]; // vD_idx could be equal to vA_idx or vB_idx
            
        SUBCASE("vdiv with mask") {
            // vdiv
            instructions::vdiv(cpuState, mem, /*v*/ vD_idx, /*v*/ vA_idx, /*v*/ vB_idx, mask);
            cout << cpuState.v[vD_idx] << endl;
            cout << mask._sgn_inner() << endl;
            CHECK(cpuState.v[vD_idx][0] == doctest::Approx(mask.bit(0) ? vD[0] : vD_before[0]));
            CHECK(cpuState.v[vD_idx][1] == doctest::Approx(mask.bit(1) ? vD[1] : vD_before[1]));
            CHECK(cpuState.v[vD_idx][2] == doctest::Approx(mask.bit(2) ? vD[2] : vD_before[2]));
            CHECK(cpuState.v[vD_idx][3] == doctest::Approx(mask.bit(3) ? vD[3] : vD_before[3]));
        }
    }

}
