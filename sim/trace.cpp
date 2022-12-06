#include "trace.h"

#include <fmt/ostream.h>

#include <morph/decoder.h>

#define INDENT "    "

std::ostream& operator<<(std::ostream& os, const InstructionTrace& trace) {
    fmt::print(os, "*** {:#x}: {:#x} ***\n", trace.pc, trace.ir);
    os << "    inputs:";
    for (auto& s : trace.inputs)
        os << " " << s;
    os << '\n';

    if (trace.vectorMask)
        fmt::print(os, INDENT "vector_mask: {:#b}\n", trace.vectorMask->raw());

    if (trace.condcode)
        os << INDENT "branch_condition_code: " << *trace.condcode << '\n';

    if (trace.controlFlow)
        os << INDENT "control_flow: " << *trace.controlFlow << '\n';

    if (trace.flagsWriteback)
        os << INDENT "flag_writeback: " << *trace.flagsWriteback << '\n';

    if (trace.scalarRegOutput)
        os << INDENT "scalar_writeback: " << *trace.scalarRegOutput << '\n';

    if (trace.vectorRegOutput)
        os << INDENT "vector_writeback: " << *trace.vectorRegOutput << '\n';

    if (trace.scalarLoad)
        os << INDENT "scalar_load: " << *trace.scalarLoad << '\n';

    if (trace.scalarStore)
        os << INDENT "scalar_store: " << *trace.scalarStore << '\n';

    if (trace.vectorLoad)
        os << INDENT "vector_load: " << *trace.vectorLoad << '\n';

    if (trace.vectorStore)
        os << INDENT "vector_store: " << *trace.vectorStore << '\n';

    os << "    asm: ";
    isa::PrintVisitor printvis(os);
    isa::decodeInstruction(printvis, bits<32>(trace.ir));
    os << '\n';

    return os;
}
