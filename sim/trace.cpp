#include "trace.h"

#include <fmt/ostream.h>

#include <morph/decoder.h>

std::ostream& operator<<(std::ostream& os, const InstructionTrace& trace) {
    fmt::print(os, "*** {:#x}: {:#x} ***\n", trace.pc, trace.ir);
    os << "    inputs:";
    for (auto& s : trace.inputs)
        os << " " << s;
    os << '\n';

    if (trace.condcode)
        os << "    branch_condition_code: " << *trace.condcode << '\n';

    if (trace.scalarRegOutput)
        os << "    scalar_writeback: " << *trace.scalarRegOutput << '\n';

    if (trace.vectorRegOutput)
        os << "    scalar_writeback: " << *trace.vectorRegOutput << '\n';

    os << "    asm: ";
    isa::PrintVisitor printvis(os);
    isa::decodeInstruction(printvis, bits<32>(trace.ir));
    os << '\n';

    return os;
}
