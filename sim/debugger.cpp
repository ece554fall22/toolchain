#include "debugger.h"

#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <linenoise.h>

#include "cpu.h"
#include <morph/bit_cast.h>
#include <morph/decoder.h>

auto split(const std::string& str, char delim = ' ')
    -> std::vector<std::string> {
    std::vector<std::string> elems;

    std::stringstream stream(str);
    std::string item;
    while (getline(stream, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}

auto parsecmd(std::string&& line) -> std::optional<Command> {
    auto els = split(line, ' ');
    if (els.empty()) {
        return std::nullopt;
    }

    auto name = els[0];
    std::vector<std::string> args(els.begin() + 1, els.end());

    return Command{
        name,
        args,
    };
}

auto Debugger::getcmd() -> std::optional<Command> {
    char* rawLine;
    if ((rawLine = linenoise("dbg> ")) != nullptr) {
        linenoiseHistoryAdd(rawLine);
        // copy so this gets cleaned up on scope. wasteful clone but better than
        // leaking or something
        std::string line(rawLine);
        linenoiseFree(rawLine);

        return parsecmd(std::move(line));
    }
    return std::nullopt;
}

void Debugger::tick() {
    while (enabled) {
        if (auto command = getcmd()) {
            dispatch(*command);
        } else
            break;
    }
    enabled = false;
}

void dumpatpc(MemSystem& mem, uint64_t breakpc) {
    isa::PrintVisitor printvis(std::cout);

    constexpr uint64_t BAND = 1;
    auto loPc = std::max(breakpc - BAND * 4, (uint64_t)0);
    auto hiPc = std::min(breakpc + BAND * 4, mem.size());
    for (uint32_t pc = loPc; pc < hiPc + 4; pc += 4) {
        if (pc == breakpc)
            std::cout << "-> ";
        else
            std::cout << "   ";
        fmt::print("{:#x}: ", pc);
        auto ir = bits<32>(mem.readInstruction(pc));
        isa::decodeInstruction(printvis, ir);
        std::cout << '\n';
    }
}

void Debugger::hitBreakpoint(bits<25> signal) {
    fmt::print("{:#x}: breaking at BKPT (signaled {:#x})\n",
               cpu.pc.getCurrentPC(), signal.inner);
    dumpatpc(mem, cpu.pc.getCurrentPC());

    enabled = true;
}

void Debugger::dispatch(Command& cmd) {
    if (cmd.name == "help" || cmd.name == "h") {
        return cmd_help(cmd);
    } else if (cmd.name == "l") {
        return cmd_list(cmd);
    } else if (cmd.name == "r") {
        return cmd_registers_scalar(cmd);
    } else if (cmd.name == "v") {
        return cmd_registers_vector(cmd);
    } else if (cmd.name == "mat") {
        return cmd_registers_mat(cmd);
    } else if (cmd.name == "flags" || cmd.name == "f") {
        return cmd_flags(cmd);
    } else if (cmd.name == "c") {
        enabled = false;
        return;
    } else if (cmd.name == "mr/32" || cmd.name == "mr/36" ||
               cmd.name == "mr/f32" || cmd.name == "mr/vec") {
        return cmd_mr(cmd);
    } else if (cmd.name == "mx/32" || cmd.name == "mx/36" ||
               cmd.name == "mx/f32" || cmd.name == "mx/vec") {
        return cmd_mx(cmd);
    } else {
        std::cerr << "command " << cmd.name << " was not recognized.\n";
    }
}

void Debugger::cmd_help(Command& cmd) {
    std::cout << "debugger help\n"
                 "--------------------------------------------------\n"
                 " l               : show current PC and instruction\n"
                 " f(lags)         : dump flags\n"
                 " r               : dump scalar registers\n"
                 " v               : dump vector registers\n"
                 " mat             : dump matrix registers in systolic core\n"
                 " mr/32  addr     : dump 32-bit value at addr\n"
                 " mr/36  addr     : dump 36-bit value at addr\n"
                 " mr/f32 addr     : dump float  value at addr\n"
                 " mr/vec addr     : dump vector value at addr\n"
                 " mx/32  addr len : dump len 32-bit values at addr\n"
                 " mx/36  addr len : dump len 36-bit values at addr\n"
                 " mx/f32 addr len : dump len float  values at addr\n"
                 " mx/vec addr len : dump len vector values at addr\n"
                 " c (or ^D)       : continue execution\n"
                 " h(elp)          : print this message\n\n";
}

void Debugger::cmd_registers_scalar(Command& command) {
    for (size_t i = 0; i < ScalarRegisterFile::N_REGS; i++) {
        auto reg = cpu.r[i];
        std::cout << std::setw(18)
                  << fmt::format("r{}: {:#011x}\n", i, reg.inner);
    }
}

void Debugger::cmd_registers_vector(Command& command) {
    for (size_t i = 0; i < VectorRegisterFile::N_REGS; i++) {
        auto reg = cpu.v[i];
        std::cout << std::setw(4) << fmt::format("v{}", i) << ": " << reg
                  << '\n';
    }
}

void Debugger::cmd_registers_mat(Command& command) {
    Eigen::IOFormat f(Eigen::StreamPrecision, 0, " ", "\n", "  ");
    std::cout << "A:\n" << cpu.matUnit.A.format(f) << "\n";
    std::cout << "B:\n" << cpu.matUnit.B.format(f) << "\n";
    std::cout << "C:\n" << cpu.matUnit.C.format(f) << "\n";
}

void Debugger::cmd_list(Command& command) {
    auto breakpc = cpu.pc.getCurrentPC();
    dumpatpc(mem, breakpc);
}

void Debugger::cmd_flags(Command& command) {
    std::cout << "flags: " << cpu.f << "\n\n";
}

auto parse_addr(std::vector<std::string>& args) -> std::optional<uint64_t> {
    if (args.empty())
        return std::nullopt;

    auto sAddr = std::string_view(args[0]);

    std::from_chars_result res{};
    uint64_t val;
    if (sAddr.starts_with("0x")) {
        sAddr.remove_prefix(2);
        res =
            std::from_chars(sAddr.data(), sAddr.data() + sAddr.size(), val, 16);
    } else {
        res =
            std::from_chars(sAddr.data(), sAddr.data() + sAddr.size(), val, 10);
    }

    args.erase(args.begin());
    if (res.ec == std::errc{} && res.ptr == (sAddr.data() + sAddr.size())) {
        return val;
    } else {
        return std::nullopt;
    }
}

void Debugger::cmd_mr(Command& cmd) {
    std::string_view n{cmd.name};
    std::vector<std::string> argstack{cmd.args};

    n.remove_prefix(3); // strip 'mr/'

    if (auto addr = parse_addr(argstack)) {
        if (n == "32") {
            uint32_t val = mem.read32(*addr);
            fmt::print("{:#011x} : {:#010x}\n", *addr, val);
        } else if (n == "36") {
            uint32_t val = mem.read32(*addr);
            fmt::print("{:#011x} : {:#010x}\n", *addr, val);
        } else if (n == "f32") {
            uint32_t val = mem.read32(*addr);
            fmt::print("{:#011x} : {}\n", *addr, bit_cast<float>(val));
        } else if (n == "vec") {
            f32x4 vec = mem.readVec(*addr);
            fmt::print("{:#011x} : {}\n", *addr, vec);
        } else {
            panic();
        }
    } else {
        fmt::print(std::cerr, "usage: {} <address>\n", cmd.name);
    }
}

void Debugger::cmd_mx(Command& cmd) {
    std::string_view n{cmd.name};
    std::vector<std::string> argstack{cmd.args};

    n.remove_prefix(3); // strip 'mx/'

    auto addr = parse_addr(argstack);
    auto len = parse_addr(argstack);

    uint64_t stride;
    if (n == "32" || n == "f32") {
        stride = 4; // 32 bits
    } else if (n == "36") {
        stride = 8; // 64 bits
    } else if (n == "vec") {
        stride = 16; // 128 bits
    } else {
        panic();
    }

    if (addr && len) {
        for (uint64_t ptr = *addr; ptr < *addr + (*len) * stride;
             ptr += stride) {
            if (n == "32") {
                uint32_t val = mem.read32(ptr);
                fmt::print("{:#011x} : {:#010x}\n", ptr, val);
            } else if (n == "36") {
                uint32_t val = mem.read32(ptr);
                fmt::print("{:#011x} : {:#010x}\n", ptr, val);
            } else if (n == "f32") {
                uint32_t val = mem.read32(ptr);
                fmt::print("{:#011x} : {}\n", ptr, bit_cast<float>(val));
            } else if (n == "vec") {
                f32x4 vec = mem.readVec(ptr);
                fmt::print("{:#011x} : {}\n", ptr, vec);
            } else {
                panic();
            }
        }
    } else {
        fmt::print(std::cerr, "usage: {} <address> <len>\n", cmd.name);
    }
}