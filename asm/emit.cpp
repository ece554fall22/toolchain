#include "emit.h"

#include <functional>

#define EMIT_NOARGS(mnemonic) [](auto& e, const auto& i) { e.mnemonic(); }

static const std::map<
    std::string, std::function<void(isa::Emitter&, const ast::Instruction&)>,
    std::less<>>
    INSTRUCTION_EMITTERS = {
        {"nop", EMIT_NOARGS(nop)},
        {"halt", EMIT_NOARGS(nop)},

        {"add",
         [](auto& e, const auto& i) {
             e.scalarArithmetic(
                 isa::ScalarArithmeticOp::Add, i.operands[0].asRegIdx(),
                 i.operands[1].asRegIdx(), i.operands[2].asRegIdx());
         }},

        {"rcsr",
         [](auto& e, const auto& i) {
             e.csr(
                 isa::CsrOp::Rcsr, i.operands[0].asRegIdx(),
                 u<2>(i.operands[1].template get<ast::OperandImmediate>().val));
         }},

        {"flushicache", EMIT_NOARGS(nop)},
};

void EmissionPass::enter(const ast::Instruction& inst, size_t depth) {
    auto it = INSTRUCTION_EMITTERS.find(inst.mnemonic.getLexeme());
    if (it != INSTRUCTION_EMITTERS.end()) {
        auto em = it->second;
        em(this->emitter, inst);
    } else {
        error(fmt::format("we don't know how to emit `{}`",
                          inst.mnemonic.getLexeme()));
    }
}
