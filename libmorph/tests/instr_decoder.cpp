#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "morph/decoder.h"

#include <array>

TEST_CASE("smoketest spinloop") {
    std::array<uint32_t, 2> code = {
        0x2000000, // nop
        0x5fffffe, // jmp #-2 ; PC' <- PC + 4 - 2 * 4 = PC - 4
    };

    isa::PrintVisitor v;

    for (size_t i = 0; i < code.size(); i++) {
        isa::decodeInstruction(v, code[i]);
        // CHECK(false);
    }
}
