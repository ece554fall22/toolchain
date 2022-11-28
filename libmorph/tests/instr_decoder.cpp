#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "morph/decoder.h"
#include "morph/disasm.h"

#include <array>

using isa::disasm::Opcode;

TEST_CASE("spinloop") {
    std::array<uint32_t, 2> code = {
        0x2000000, // nop
        0x5fffffe, // jmp #-2 ; PC' <- PC + 4 - 2 * 4 = PC - 4
    };

    isa::disasm::DisasmVisitor dv;

    isa::decodeInstruction(dv, code[0]);
    CHECK(dv.getInstr().opcode == Opcode::Nop);

    isa::decodeInstruction(dv, code[1]);
    CHECK(dv.getInstr().opcode == Opcode::Jmp);
    CHECK(dv.getInstr().imm == -2);
}
