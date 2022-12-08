#include "sema.h"

#include <functional>
#include <iostream>
#include <map>
#include <string>

#include <fmt/core.h>
#include <fmt/ostream.h>

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

    {"vldi", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::VectorMemoryImmIncr}},
    {"vsti", {OperandType::VectorMask, OperandType::VectorMemoryImmIncr,   OperandType::VectorRegister}},
    {"vldr", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::VectorMemoryRegIncr}},
    {"vstr", {OperandType::VectorMask, OperandType::VectorMemoryRegIncr,   OperandType::VectorRegister}},


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
    RRR("cmpdec"),
    RRR("cmpinc"),

    RRR("fadd"),
    RRR("fsub"),
    RRR("fmul"),
    RRR("fdiv"),

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

    {"vidx", {OperandType::ScalarRegister, OperandType::VectorRegister, OperandType::Immediate}},
    {"vsplat", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::ScalarRegister}},
    {"vswizzle", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::VectorRegister, OperandType::Immediate, OperandType::Immediate, OperandType::Immediate, OperandType::Immediate}},
    {"vreduce", {OperandType::ScalarRegister, OperandType::VectorMask, OperandType::VectorRegister}},
    {"vdot", {OperandType::ScalarRegister, OperandType::VectorRegister, OperandType::VectorRegister}},
    {"vdota", {OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::VectorRegister, OperandType::VectorRegister}},
    {"vsma", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::ScalarRegister, OperandType::VectorRegister, OperandType::VectorRegister}},
    {"vcomp", {OperandType::VectorMask, OperandType::VectorRegister, OperandType::ScalarRegister, OperandType::ScalarRegister, OperandType::VectorRegister}},

    {"rcsr", {OperandType::ScalarRegister, OperandType::Immediate}},
    {"wcsr", {OperandType::Immediate,      OperandType::ScalarRegister}},

    {"writeA", {OperandType::VectorRegister, OperandType::VectorRegister, OperandType::Immediate}},
    {"writeB", {OperandType::VectorRegister, OperandType::VectorRegister, OperandType::Immediate}},
    {"writeC", {OperandType::VectorRegister, OperandType::VectorRegister, OperandType::Immediate}},
    {"readC", {OperandType::VectorRegister, OperandType::Immediate, OperandType::Immediate}},
    {"systolicstep", {}},

    {"flushicache", {}},
    {"flushdirty", {}},
    {"flushclean", {}},
    {"flushline", {OperandType::ScalarRegister, OperandType::Immediate}},

    {"ftoi", {OperandType::ScalarRegister, OperandType::ScalarRegister}},
    {"itof", {OperandType::ScalarRegister, OperandType::ScalarRegister}},

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
                if (!operand.is<ast::OperandMemory>()) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected memory operand",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::VectorMemoryImmIncr:
                if (!operand.is<ast::OperandMemoryPostIncr>() ||
                    !std::holds_alternative<ast::OperandImmediate>(
                        operand.get<ast::OperandMemoryPostIncr>().increment)) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected vector memory "
                                      "operand w/ immediate increment",
                                      i, inst.mnemonic.getLexeme()));
                }
                break;
            case OperandType::VectorMemoryRegIncr:
                if (!operand.is<ast::OperandMemoryPostIncr>() ||
                    !std::holds_alternative<ast::OperandRegister>(
                        operand.get<ast::OperandMemoryPostIncr>().increment)) {
                    error(inst.mnemonic.getSrcLoc()->lineno,
                          fmt::format("operand {} to instruction `{}` has "
                                      "wrong type; expected vector memory "
                                      "operand w/ register increment",
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

void SemanticsPass::dumpSemanticsTable(std::ostream& os) {
    for (auto& [mnemonic, args] : SEMANTICS) {
        os << "---- " << mnemonic << " ----\n";
        os << "  semantics: " << mnemonic << " ";
        size_t n_args = args.size();
        for (size_t i = 0; i < n_args; i++) {
            switch (args[i]) {
            case OperandType::ScalarRegister:
                os << "scalar";
                break;
            case OperandType::VectorRegister:
                os << "vector";
                break;
            case OperandType::VectorMask:
                os << "mask";
                break;
            case OperandType::Immediate:
                os << "imm";
                break;
            case OperandType::Label:
                os << "lbl";
                break;
            case OperandType::Memory:
                os << "mem";
                break;
            case OperandType::VectorMemoryImmIncr:
                os << "vmem+=imm";
                break;
            case OperandType::VectorMemoryRegIncr:
                os << "vmem+=reg";
                break;
            }

            if (i < n_args - 1)
                os << ", ";
        }
        os << "\n";

        os << "  example:   " << mnemonic << " ";
        size_t reg_stub = 0;
        for (size_t i = 0; i < n_args; i++) {
            switch (args[i]) {
            case OperandType::ScalarRegister:
                fmt::print(os, "r{}", reg_stub);
                reg_stub++;
                break;
            case OperandType::VectorRegister:
                fmt::print(os, "v{}", reg_stub);
                reg_stub++;
                break;
            case OperandType::VectorMask:
                os << "0b0101";
                break;
            case OperandType::Immediate:
                os << "0x20";
                break;
            case OperandType::Label:
                os << "someLabel";
                break;
            case OperandType::Memory:
                fmt::print(os, "[r{}+0x10]", reg_stub);
                reg_stub++;
                break;
            case OperandType::VectorMemoryImmIncr:
                fmt::print(os, "[r{}+=0x10]", reg_stub);
                reg_stub++;
                break;
            case OperandType::VectorMemoryRegIncr:
                fmt::print(os, "[r{}+=r{}]", reg_stub, reg_stub + 1);
                reg_stub += 2;
                break;
            }

            if (i < n_args - 1)
                os << ", ";
        }

        os << "\n\n";
    }
}