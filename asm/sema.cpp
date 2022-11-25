#include "sema.h"

#include <map>
#include <string>

enum class OperandType {
    Register,
    Immediate,
};

static const std::map<std::string, std::vector<OperandType>> SEMANTICS = {
    {"nop", {}},  {"halt", {}},

    {"jmp", {}},  {"jal", {}},  {"jmpr", {}}, {"jalr", {}},

    {"bnzi", {}}, {"bezi", {}}, {"blzi", {}}, {"bgzi", {}},
    {"blei", {}}, {"bgei", {}},

    {"bnzr", {}}, {"bezr", {}}, {"blzr", {}}, {"bgzr", {}},
    {"bler", {}}, {"bger", {}},
};

void SemanticsPass::enter(const ast::Instruction& inst, size_t depth) {
    auto it = SEMANTICS.find(std::string(inst.mnemonic.getLexeme()));
    if (it != SEMANTICS.end()) {

    } else {
        error(inst.mnemonic.getSrcLoc()->lineno,
              fmt::format("unrecognized instruction mnemonic `{}`",
                          inst.mnemonic.getLexeme()));
    }
}
