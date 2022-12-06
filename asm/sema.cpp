#include "sema.h"

#include <functional>
#include <map>
#include <string>

#include <iostream>

#define RRR(id)                                                                \
    {                                                                          \
        id, {                                                                  \
            OperandType::ScalarRegister, OperandType::ScalarRegister,          \
                OperandType::ScalarRegister                                    \
        }                                                                      \
    }
#define RRI(id)                                                                \
    {                                                                          \
        id, {                                                                  \
            OperandType::ScalarRegister, OperandType::ScalarRegister,          \
                OperandType::Immediate                                         \
        }                                                                      \
    }
#define MVVV(id)                                                               \
    {                                                                          \
        id, {                                                                  \
            OperandType::VectorMask, OperandType::VectorRegister,              \
                OperandType::VectorRegister, OperandType::VectorRegister       \
        }                                                                      \
    }
#define MVVR(id)                                                               \
    {                                                                          \
        id, {                                                                  \
            OperandType::VectorMask, OperandType::VectorRegister,              \
                OperandType::VectorRegister, OperandType::ScalarRegister       \
        }                                                                      \
    }

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

    {"bnzr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"bezr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"blzr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"bgzr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"bler", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"bger", {OperandType::ScalarRegister, OperandType::Immediate}},

    {"lih",  {OperandType::ScalarRegister, OperandType::Immediate}},
    {"lil",  {OperandType::ScalarRegister, OperandType::Immediate}},

    {"ld32", {OperandType::ScalarRegister, OperandType::Memory}},
    {"ld36", {OperandType::ScalarRegister, OperandType::Memory}},
    {"st32", {OperandType::Memory,         OperandType::ScalarRegister}},
    {"st36", {OperandType::Memory,         OperandType::ScalarRegister}},

    RRI("addi"),
    RRI("subi"),
    RRI("andi"),
    RRI("ori"),
    RRI("xori"),
    RRI("shli"),
    RRI("shri"),

    {"cmpi", {OperandType::ScalarRegister, OperandType::Immediate}},

    RRR("add"),
    RRR("sub"),
    RRR("mul"),
    RRR("and"),
    RRR("or"),
    RRR("xor"),
    RRR("shr"),
    RRR("shl"),
    {"not", {OperandType::ScalarRegister, OperandType::ScalarRegister}},

    {"cmp",  {OperandType::ScalarRegister, OperandType::ScalarRegister}},

    MVVV("vadd"),
    MVVV("vsub"),
    MVVV("vmul"),
    MVVV("vdiv"),
    MVVV("vmax"),
    MVVV("vmin"),

    MVVR("vsadd"),
    MVVR("vssub"),
    MVVR("vsmul"),
    MVVR("vsdiv"),

    {"vidx", {OperandType::ScalarRegister, OperandType::VectorRegister, OperandType::Immediate}}, // todo
    {"vsplat", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::ScalarRegister}},

    {"rcsr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"wcsr", {OperandType::Immediate,      OperandType::ScalarRegister}},

    {"flushicache", {}},
    {"flushdirty", {}},
    {"flushclean", {}},
    {"flushline", {OperandType::ScalarRegister, OperandType::Immediate}},

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
                if (!operand.is<ast::OperandRegister>() ||
                    operand.get<ast::OperandRegister>().vector) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format(
                              "operand {} to instruction `{}` has "
                              "wrong type; expected scalar register operand",
                              i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::VectorRegister:
                if (!operand.is<ast::OperandRegister>() ||
                    !operand.get<ast::OperandRegister>().vector) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format(
                              "operand {} to instruction `{}` has "
                              "wrong type; expected vector register operand",
                              i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::VectorMask: {
                if (!operand.is<ast::OperandImmediate>()) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected vector mask",
                                      i, inst.mnemonic.getLexeme()));
                    break;
                }
                auto val = operand.get<ast::OperandImmediate>().val;
                if (val < 0 || val > 0b1111) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("vector mask to instruction `{}` must be "
                                      "a 4-bit bitfield",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            }
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
                if (!operand.is<ast::OperandMemory>() ||
                    operand.get<ast::OperandMemory>().increment) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected memory operand",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::VectorMemory:
                if (!operand.is<ast::OperandMemory>() ||
                    !operand.get<ast::OperandMemory>().increment) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format(
                              "operand {} to instruction `{}` has "
                              "wrong type; expected vector memory operand",
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
