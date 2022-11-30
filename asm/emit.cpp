#include "emit.h"

#include <functional>
#include <morph/encoder.h>

void emit_arith(isa::ScalarArithmeticOp op, isa::Emitter& e,
                const ast::Instruction& i) {
    e.scalarArithmetic(op, i.operands[0].asRegIdx(), i.operands[1].asRegIdx(),
                       i.operands[2].asRegIdx());
}

void emit_arith_imm(isa::ScalarArithmeticOp op, isa::Emitter& e,
                    const ast::Instruction& i) {
    e.scalarArithmeticImmediate(op, i.operands[0].asRegIdx(),
                                i.operands[1].asRegIdx(),
                                i.operands[2].template asBitsImm<15>());
}

void emit_flushcache(isa::CacheControlOp op, isa::Emitter& e,
                     const ast::Instruction& i) {
    e.flushcache(op);
}

// todo this is just fucked up std::bind but with a defined retn ty
#define PARTIAL(fn, ...) [](auto& e, const auto& i) { fn(__VA_ARGS__, e, i); }
#define EMIT_NOARGS(mnemonic) [](auto& e, const auto& i) { e.mnemonic(); }

static const std::map<
    std::string, std::function<void(isa::Emitter&, const ast::Instruction&)>,
    std::less<>>
    INSTRUCTION_EMITTERS = {
        {"nop", EMIT_NOARGS(nop)},
        {"halt", EMIT_NOARGS(nop)},

        {"bkpt",
         [](auto& e, const ast::Instruction& i) {
             e.bkpt(i.operands[0].asBitsImm<25>());
         }},

        {"lil",
         [](isa::Emitter& e, const ast::Instruction& i) {
             e.loadImmediate(false, i.operands[0].asRegIdx(),
                             i.operands[1].asSignedImm<18>());
         }},
        {"lih",
         [](isa::Emitter& e, const ast::Instruction& i) {
             e.loadImmediate(true, i.operands[0].asRegIdx(),
                             i.operands[1].asSignedImm<18>());
         }},

        {"ld32",
         [](isa::Emitter& e, const ast::Instruction& i) {
             auto memOp = i.operands[1].get<ast::OperandMemory>();
             assert(!memOp.increment); // TODO!

             e.loadScalar(false, i.operands[0].asRegIdx(), memOp.base.idx,
                          memOp.offset);
         }},

        {"ld36",
         [](isa::Emitter& e, const ast::Instruction& i) {
             auto memOp = i.operands[1].get<ast::OperandMemory>();
             assert(!memOp.increment); // TODO!

             e.loadScalar(true, i.operands[0].asRegIdx(), memOp.base.idx,
                          memOp.offset);
         }},

        {"addi", PARTIAL(emit_arith_imm, isa::ScalarArithmeticOp::Add)},
        {"subi", PARTIAL(emit_arith_imm, isa::ScalarArithmeticOp::Sub)},
        {"andi", PARTIAL(emit_arith_imm, isa::ScalarArithmeticOp::And)},
        {"xori", PARTIAL(emit_arith_imm, isa::ScalarArithmeticOp::Xor)},
        {"shri", PARTIAL(emit_arith_imm, isa::ScalarArithmeticOp::Shr)},
        {"shli", PARTIAL(emit_arith_imm, isa::ScalarArithmeticOp::Shl)},

        // TODO: can we force std::bind to work here despite undef retn ty?
        {"add", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Add)},
        {"sub", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Sub)},
        {"mul", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Mul)},
        {"and", PARTIAL(emit_arith, isa::ScalarArithmeticOp::And)},
        {"or",  PARTIAL(emit_arith, isa::ScalarArithmeticOp::Or)},
        {"xor", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Xor)},
        {"shr", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Shr)},
        {"shl", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Shl)},

        {"rcsr",
         [](auto& e, const ast::Instruction& i) {
             e.csr(isa::CsrOp::Rcsr, i.operands[0].asRegIdx(),
                   u<2>(i.operands[1].get<ast::OperandImmediate>().val));
         }},

        {"flushicache",
         PARTIAL(emit_flushcache, isa::CacheControlOp::Flushicache)},
        {"flushdirty",
         PARTIAL(emit_flushcache, isa::CacheControlOp::Flushdirty)},
        {"flushclean",
         PARTIAL(emit_flushcache, isa::CacheControlOp::Flushclean)},
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
