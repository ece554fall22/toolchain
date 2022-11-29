#include "emit.h"

#include <functional>

#define EMIT_NOARGS(mnemonic) [](auto& e, const auto& i) { e.mnemonic(); }

void emit_arith(isa::ScalarArithmeticOp op, isa::Emitter& e,
                const ast::Instruction& i) {
    e.scalarArithmetic(op, i.operands[0].asRegIdx(), i.operands[1].asRegIdx(),
                       i.operands[2].asRegIdx());
}

void emit_arith_imm(isa::ScalarArithmeticOp op, isa::Emitter& e,
                    const ast::Instruction& i) {
    e.scalarArithmeticImmediate(op, i.operands[0].asRegIdx(),
                                i.operands[1].asRegIdx(),
                                i.operands[2].template asSignedImm<15>());
}

// todo this is just fucked up std::bind but with a defined retn ty
#define PARTIAL(fn, ...) [](auto& e, const auto& i) { fn(__VA_ARGS__, e, i); }

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

        // TODO: can we force std::bind to work here despite undef retn ty?
        {"add", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Add)},
        {"sub", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Sub)},
        {"mul", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Mul)},
        {"mul", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Mul)},
        {"and", PARTIAL(emit_arith, isa::ScalarArithmeticOp::And)},
        {"or", PARTIAL(emit_arith,  isa::ScalarArithmeticOp::Or)},
        {"xor", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Xor)},
        {"shr", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Shr)},
        {"shl", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Shl)},

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
