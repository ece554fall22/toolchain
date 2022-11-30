#include "trace.h"

#include <fmt/ostream.h>

std::ostream &operator<<(std::ostream &os, const InstructionTrace &trace) {
    fmt::print(os, "*** {:#x}: {:#x} ***\n", trace.pc, trace.ir);

    return os;
}
