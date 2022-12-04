#include "sema.h"

#include <functional>
#include <map>
#include <string>

#include <iostream>

// clang-format off
static const std::map<std::string, std::vector<OperandType>, std::less<>> SEMANTICS = {
    {"nop",  {}},
    {"halt", {}},
    {"bkpt", {OperandType::Immediate}},

    {"jmp",  {OperandType::Label}},
    {"jal",  {OperandType::Label}},
    {"jmpr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"jalr", {OperandType::ScalarRegister, OperandType::Immediate}},

    {"bnzi", {OperandType::Label}},
    {"bezi", {OperandType::Label}},
    {"blzi", {OperandType::Label}},
    {"bgzi", {OperandType::Label}},
    {"blei", {OperandType::Label}},
    {"bgei", {OperandType::Label}},

    {"bnzr", {OperandType::Register, OperandType::Immediate}},
    {"bezr", {OperandType::Register, OperandType::Immediate}},
    {"blzr", {OperandType::Register, OperandType::Immediate}},
    {"bgzr", {OperandType::Register, OperandType::Immediate}},
    {"bler", {OperandType::Register, OperandType::Immediate}},
    {"bger", {OperandType::Register, OperandType::Immediate}},

    {"lih",  {OperandType::ScalarRegister, OperandType::Immediate}},
    {"lil",  {OperandType::ScalarRegister, OperandType::Immediate}},

    {"ld32", {OperandType::ScalarRegister, OperandType::Memory}},
    {"ld36", {OperandType::ScalarRegister, OperandType::Memory}},
    {"st32", {OperandType::Memory,         OperandType::ScalarRegister}},
    {"st36", {OperandType::Memory,         OperandType::ScalarRegister}},

    {"addi", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},
    {"subi", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},
    {"andi", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},
    {"ori",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},
    {"xori", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},
    {"shli", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},
    {"shri", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::Immediate}},

    {"cmpi", {OperandType::ScalarRegister, OperandType::Immediate}},

{"add",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"sub",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"mul",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"and",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"or",   {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"xor",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"shr",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"shl",  {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::ScalarRegister}},
{"not",  {OperandType::ScalarRegister, OperandType::ScalarRegister}},

    {"cmp",  {OperandType::ScalarRegister, OperandType::ScalarRegister}},

    {"vadd", {OperandType::Register}}

    {"rcsr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"wcsr", {OperandType::Immediate,      OperandType::ScalarRegister}},

    {"flushicache", {}},
    {"flushdirty", {}},
    {"flushclean", {}},
    {"flushline", {OperandType::Register, OperandType::Immediate}},

    // pseudos
    {"lda",  {OperandType::ScalarRegister, OperandType::Label}},

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
            case OperandType::ScalarRegister:
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
