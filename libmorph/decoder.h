#pragma once

#include "ty.h"
#include "util.h"
#include "varint.h"
#include <cstdint>

namespace isa {

struct InstructionVisitor {
    virtual ~InstructionVisitor() = default;

    virtual void nop() = 0;
    virtual void halt() = 0;

    virtual void jmp(s<25> to) = 0;
    virtual void jal(s<25> to) = 0;
};

struct PrintVisitor : public InstructionVisitor {
    virtual ~PrintVisitor() = default;

    virtual void nop() { std::cout << "nop\n"; }
    virtual void halt() { std::cout << "halt\n"; }
    virtual void jmp(s<25> to) { std::cout << "jmp " << to << "\n"; }
    virtual void jal(s<25> to) { std::cout << "jal " << to << "\n"; }
};

void decodeJ(InstructionVisitor& visit, uint32_t instr);
void decodeInstruction(InstructionVisitor& visit, uint32_t instr);

} // namespace isa
