#include "sema.h"

#include <functional>
#include <map>
#include <string>

#include <iostream>

// clang-format off
static const std::map<std::string, std::vector<OperandType>, std::less<>> SEMANTICS = {
    {"nop",  {}},
    {"halt", {}},

    {"jmp",  {OperandType::Label}},
    {"jal",  {OperandType::Label}},
    {"jmpr", {OperandType::Register, OperandType::Immediate}},
    {"jalr", {OperandType::Register, OperandType::Immediate}},

    {"bnzi", {OperandType::Label}},
    {"bezi", {OperandType::Label}},
    {"blzi", {OperandType::Label}},
    {"bgzi", {OperandType::Label}},
    {"blei", {OperandType::Label}},
    {"bgei", {OperandType::Label}},

    {"bnzr", {}},
    {"bezr", {}},
    {"blzr", {}},
    {"bgzr", {}},
    {"bler", {}},
    {"bger", {}},

    {"lih",  {OperandType::Register, OperandType::Immediate}},
    {"lil",  {OperandType::Register, OperandType::Immediate}},

    {"ld32", {OperandType::Register, OperandType::Memory}},
    {"ld36", {OperandType::Register, OperandType::Memory}},
    {"st32", {OperandType::Memory,   OperandType::Register}},
    {"st36", {OperandType::Memory,   OperandType::Register}},

    {"addi", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"subi", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"andi", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"ori",  {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"xori", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"shli", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"shri", {OperandType::Register, OperandType::Register, OperandType::Immediate}},

    {"cmpi", {OperandType::Register, OperandType::Immediate}},

    {"addi", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"subi", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"andi", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"ori",  {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"xori", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"shli", {OperandType::Register, OperandType::Register, OperandType::Immediate}},
    {"shri", {OperandType::Register, OperandType::Register, OperandType::Immediate}},

    {"add",  {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"sub",  {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"mult", {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"and",  {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"or",   {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"xor",  {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"shr",  {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"shl",  {OperandType::Register, OperandType::Register, OperandType::Register}},
    {"not",  {OperandType::Register, OperandType::Register}},

    {"rcsr", {OperandType::Register, OperandType::Immediate}},
    {"wcsr", {OperandType::Immediate, OperandType::Register}},

    {"flushicache", {}},

    // pseudos
    {"lda",  {OperandType::Register, OperandType::Label}},

    // data
    {"dw36", {OperandType::Immediate}},
    {"dw32", {OperandType::Immediate}},
};
// clang-format on

void SemanticsPass::enter(const ast::Instruction& inst, size_t depth) {
    auto it = SEMANTICS.find(inst.mnemonic.getLexeme());
    if (it != SEMANTICS.end()) {
        auto sema = it->second;

        if (sema.size() != inst.operands.size()) {
            error(
                inst.mnemonic.getSrcLoc()->lineno,
                fmt::format(
                    "instruction `{}` invoked with {} operands; we expected {}",
                    inst.mnemonic.getLexeme(), inst.operands.size(),
                    sema.size()));
        }

        size_t i = 0;
        for (const auto& operand : inst.operands) {
            switch (sema[i]) {
            case OperandType::Register:
                if (!operand.is<ast::OperandRegister>()) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected register operand",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::Immediate:
                if (!operand.is<ast::OperandImmediate>()) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected immediate operand",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::Label:
                if (!operand.is<ast::OperandLabel>()) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected label",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::Memory:
                if (!operand.is<ast::OperandMemory>()) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected memory operand",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            }

            i++;
        }
    } else {
        error(inst.mnemonic.getSrcLoc()->lineno,
              fmt::format("unrecognized instruction mnemonic `{}`",
                          inst.mnemonic.getLexeme()));
    }
}
