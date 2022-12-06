#include "emit.h"

#include <functional>
#include <morph/encoder.h>

void emit_arith(isa::ScalarArithmeticOp op, isa::Emitter& e,
                const SymbolTable& symtab, const ast::Instruction& i) {
    e.scalarArithmetic(op, i.operands[0].asRegIdx(), i.operands[1].asRegIdx(),
                       i.operands[2].asRegIdx());
}

void emit_arith_imm(isa::ScalarArithmeticOp op, isa::Emitter& e,
                    const SymbolTable& symtab, const ast::Instruction& i) {
    e.scalarArithmeticImmediate(op, i.operands[0].asRegIdx(),
                                i.operands[1].asRegIdx(),
                                i.operands[2].template asBitsImm<15>());
}

void emit_vector_lanewise(isa::LanewiseVectorOp op, isa::Emitter& e,
                          const SymbolTable& symtab,
                          const ast::Instruction& i) {
    e.vectorLanewiseArith(op, i.operands[1].asRegIdx(),
                          i.operands[2].asRegIdx(), i.operands[3].asRegIdx(),
                          i.operands[0].asBitsImm<4>());
}

void emit_vector_scalar(isa::VectorScalarOp op, isa::Emitter& e,
                        const SymbolTable& symtab, const ast::Instruction& i) {
    e.vectorScalarArith(op, i.operands[1].asRegIdx(), i.operands[3].asRegIdx(),
                        i.operands[2].asRegIdx(),
                        i.operands[0].asBitsImm<vmask_t::size>());
}

void emit_flushcache(isa::CacheControlOp op, isa::Emitter& e,
                     const SymbolTable& symtab, const ast::Instruction& i) {
    e.flushcache(op);
}

void emit_scalar_load(bool b36, isa::Emitter& e, const SymbolTable& symtab,
                      const ast::Instruction& i) {
    auto memOp = i.operands[1].get<ast::OperandMemory>();
    assert(!memOp.base.vector); // TODO

    e.loadScalar(b36, i.operands[0].asRegIdx(), memOp.base.idx, memOp.offset);
}

void emit_scalar_store(bool b36, isa::Emitter& e, const SymbolTable& symtab,
                       const ast::Instruction& i) {
    auto memOp = i.operands[0].get<ast::OperandMemory>();
    assert(!memOp.base.vector); // TODO

    e.storeScalar(b36, memOp.base.idx, i.operands[1].asRegIdx(), memOp.offset);
}

auto compute_offset(const isa::Emitter& e, const Symbol& symb) -> int64_t {
    int64_t offset = static_cast<int64_t>(symb.addr) - (e.getPC() + 4);
    //            fmt::print("PC={:#x}\n",e.getPC());
    //            fmt::print("offset={:#x}\n", offset);
    assert((offset % 4) == 0 && "alignment");
    offset /= 4; // immediate scaling

    return offset;
}

void emit_bi(condition_t cond, isa::Emitter& e, const SymbolTable& symtab,
             const ast::Instruction& i) {
    auto symb =
        symtab.get(i.operands[0].get<ast::OperandLabel>().label.getLexeme());
    assert(symb.has_value() && "undef symbol uh oh"); // TODO
    auto offset = compute_offset(e, *symb);
    e.branchImm(cond, offset);
}

void emit_br(condition_t cond, isa::Emitter& e,
             [[maybe_unused]] const SymbolTable& symtab,
             const ast::Instruction& i) {
    e.branchReg(cond, i.operands[0].asRegIdx(),
                i.operands[1].asSignedImm<17>());
}

void emit_scalar_float(isa::FloatArithmeticOp op, isa::Emitter& e,
                       const SymbolTable& symtab, const ast::Instruction& i) {
    e.floatArithmetic(op, i.operands[0].asRegIdx(), i.operands[1].asRegIdx(),
                      i.operands[2].asRegIdx());
}

// todo this is just fucked up std::bind but with a defined retn ty
#define PARTIAL(fn, ...)                                                       \
    [](auto& e, const SymbolTable& st, const auto& i) {                        \
        fn(__VA_ARGS__, e, st, i);                                             \
    }
#define EMIT_NOARGS(mnemonic)                                                  \
    [](auto& e, const SymbolTable& st, const auto& i) { e.mnemonic(); }

static const std::map<std::string,
                      std::function<void(isa::Emitter&, const SymbolTable&,
                                         const ast::Instruction&)>,
                      std::less<>>
    INSTRUCTION_EMITTERS = {
        {"nop", EMIT_NOARGS(nop)},
        {"halt", EMIT_NOARGS(halt)},

        {"bkpt",
         [](auto& e, const SymbolTable& symtab, const ast::Instruction& i) {
             e.bkpt(i.operands[0].asBitsImm<25>());
         }},

        {"jmp",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto symb = symtab.get(
                 i.operands[0].get<ast::OperandLabel>().label.getLexeme());
             assert(symb.has_value() && "undef symbol uh oh"); // TODO
             auto offset = compute_offset(e, *symb);
             e.jumpPCRel(s<25>(offset), false);
         }},

        {"jal",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto symb = symtab.get(
                 i.operands[0].get<ast::OperandLabel>().label.getLexeme());
             assert(symb.has_value() && "undef symbol uh oh"); // TODO
             auto offset = compute_offset(e, *symb);
             e.jumpPCRel(s<25>(offset), true);
         }},

        {"jmpr",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto offset = i.operands[1].get<ast::OperandImmediate>().val;
             assert((offset % 4) == 0 && "alignment"); // TODO!
             offset /= 4;                              // scale for offset imm

             e.jumpRegRel(i.operands[0].asRegIdx(), s<20>(offset), false);
         }},

        {"jalr",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto offset = i.operands[1].get<ast::OperandImmediate>().val;
             assert((offset % 4) == 0 && "alignment"); // TODO!
             offset /= 4;                              // scale for offset imm

             e.jumpRegRel(i.operands[0].asRegIdx(), s<20>(offset), true);
         }},

        {"bnzi", PARTIAL(emit_bi, condition_t::nz)},
        {"bezi", PARTIAL(emit_bi, condition_t::ez)},
        {"blzi", PARTIAL(emit_bi, condition_t::lz)},
        {"bgzi", PARTIAL(emit_bi, condition_t::gz)},
        {"blei", PARTIAL(emit_bi, condition_t::le)},
        {"bgei", PARTIAL(emit_bi, condition_t::ge)},

        {"bnzr", PARTIAL(emit_br, condition_t::nz)},
        {"bezr", PARTIAL(emit_br, condition_t::ez)},
        {"blzr", PARTIAL(emit_br, condition_t::lz)},
        {"bgzr", PARTIAL(emit_br, condition_t::gz)},
        {"bler", PARTIAL(emit_br, condition_t::le)},
        {"bger", PARTIAL(emit_br, condition_t::ge)},

        {"lil",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.loadImmediate(false, i.operands[0].asRegIdx(),
                             i.operands[1].asSignedImm<18>());
         }},
        {"lih",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.loadImmediate(true, i.operands[0].asRegIdx(),
                             i.operands[1].asSignedImm<18>());
         }},

        {"ld32", PARTIAL(emit_scalar_load, false)},
        {"ld36", PARTIAL(emit_scalar_load, true)},

        {"st32", PARTIAL(emit_scalar_store, false)},
        {"st36", PARTIAL(emit_scalar_store, true)},

        {"vldi",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto memOp = i.operands[2].get<ast::OperandMemoryPostIncr>();
             assert(!memOp.base.vector); // TODO

             auto incr = std::get<ast::OperandImmediate>(memOp.increment);
             assert((incr.val % 0x10) == 0 &&
                    "vector memory immediate increment must be 0x10-aligned");

             e.loadVectorImmStride(i.operands[1].asRegIdx(), memOp.base.idx,
                                   incr.val / 0x10,
                                   i.operands[0].asBitsImm<vmask_t::size>());
         }},
        {"vsti",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto memOp = i.operands[1].get<ast::OperandMemoryPostIncr>();
             assert(!memOp.base.vector); // TODO

             auto incr = std::get<ast::OperandImmediate>(memOp.increment);
             assert((incr.val % 0x10) == 0 &&
                    "vector memory immediate increment must be 0x10-aligned");

             e.storeVectorImmStride(memOp.base.idx, i.operands[2].asRegIdx(),
                                    incr.val / 0x10,
                                    i.operands[0].asBitsImm<vmask_t::size>());
         }},
        {"vldr",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto memOp = i.operands[2].get<ast::OperandMemoryPostIncr>();
             assert(!memOp.base.vector); // TODO

             auto incr = std::get<ast::OperandRegister>(memOp.increment);
             assert(!incr.vector); // TODO

             e.loadVectorRegStride(i.operands[1].asRegIdx(), memOp.base.idx,
                                   incr.idx,
                                   i.operands[0].asBitsImm<vmask_t::size>());
         }},
        {"vstr",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             auto memOp = i.operands[1].get<ast::OperandMemoryPostIncr>();
             assert(!memOp.base.vector); // TODO

             auto incr = std::get<ast::OperandRegister>(memOp.increment);
             assert(!incr.vector); // TODO

             e.storeVectorRegStride(memOp.base.idx, incr.idx,
                                    i.operands[2].asRegIdx(),
                                    i.operands[0].asBitsImm<vmask_t::size>());
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
        {"or", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Or)},
        {"xor", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Xor)},
        {"shr", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Shr)},
        {"shl", PARTIAL(emit_arith, isa::ScalarArithmeticOp::Shl)},
        {"not",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.scalarArithmeticNot(i.operands[0].asRegIdx(),
                                   i.operands[1].asRegIdx());
         }},

        {"cmp",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.compareReg(i.operands[0].asRegIdx(), i.operands[1].asRegIdx());
         }},

        {"cmpi",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.compareImm(i.operands[0].asRegIdx(),
                          i.operands[1].asSignedImm<20>());
         }},

        {"fadd", PARTIAL(emit_scalar_float, isa::FloatArithmeticOp::Fadd)},
        {"fsub", PARTIAL(emit_scalar_float, isa::FloatArithmeticOp::Fsub)},
        {"fmul", PARTIAL(emit_scalar_float, isa::FloatArithmeticOp::Fmul)},
        {"fdiv", PARTIAL(emit_scalar_float, isa::FloatArithmeticOp::Fdiv)},

        {"vadd", PARTIAL(emit_vector_lanewise, isa::LanewiseVectorOp::Add)},
        {"vsub", PARTIAL(emit_vector_lanewise, isa::LanewiseVectorOp::Sub)},
        {"vmul", PARTIAL(emit_vector_lanewise, isa::LanewiseVectorOp::Mul)},
        {"vdiv", PARTIAL(emit_vector_lanewise, isa::LanewiseVectorOp::Div)},
        {"vmax", PARTIAL(emit_vector_lanewise, isa::LanewiseVectorOp::Max)},
        {"vmin", PARTIAL(emit_vector_lanewise, isa::LanewiseVectorOp::Min)},

        {"vsadd", PARTIAL(emit_vector_scalar, isa::VectorScalarOp::Add)},
        {"vssub", PARTIAL(emit_vector_scalar, isa::VectorScalarOp::Sub)},
        {"vsmul", PARTIAL(emit_vector_scalar, isa::VectorScalarOp::Mul)},
        {"vsdiv", PARTIAL(emit_vector_scalar, isa::VectorScalarOp::Div)},

        {"vidx",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.vidx(i.operands[0].asRegIdx(), i.operands[1].asRegIdx(),
                    i.operands[2].asBitsImm<vlaneidx_t::size>());
         }},
        {"vsplat",
         [](isa::Emitter& e, const SymbolTable& symtab,
            const ast::Instruction& i) {
             e.vsplat(i.operands[1].asRegIdx(), i.operands[2].asRegIdx(),
                      i.operands[0].asBitsImm<vmask_t::size>());
         }},

        {"rcsr",
         [](auto& e, const SymbolTable& symtab, const ast::Instruction& i) {
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
        em(this->emitter, symtab, inst);
    } else {
        error(fmt::format("we don't know how to emit `{}`",
                          inst.mnemonic.getLexeme()));
    }
}
