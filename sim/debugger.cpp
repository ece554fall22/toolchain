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

void Debugger::hitBreakpoint() { enabled = true; }

void Debugger::dispatch(Command& cmd) {
    if (cmd.name == "help" || cmd.name == "h") {
        return cmd_help(cmd);
    } else if (cmd.name == "l") {
        return cmd_list(cmd);
    } else if (cmd.name == "r") {
        return cmd_registers_scalar(cmd);
    } else if (cmd.name == "v") {
        return cmd_registers_vector(cmd);
    } else if (cmd.name == "flags" || cmd.name == "f") {
        return cmd_flags(cmd);
    } else if (cmd.name == "c") {
        enabled = false;
        return;
    } else if (cmd.name == "mr/32") {
        return cmd_m32(cmd);
    } else if (cmd.name == "mr/36") {
        return cmd_m36(cmd);
    } else if (cmd.name == "mr/f32") {
        return cmd_f32(cmd);
    } else if (cmd.name == "mr/vec") {
        return cmd_vec(cmd);
    } else {
        std::cerr << "command " << cmd.name << " was not recognized.\n";
    }
}

void Debugger::cmd_help(Command& cmd) {
    std::cout << "debugger help\n"
                 "-------------\n"
                 " l           : show current PC and instruction\n"
                 " f(lags)     : dump flags\n"
                 " r           : dump scalar registers\n"
                 " v           : dump vector registers\n"
                 " mr/32  addr : dump 32-bit value at addr\n"
                 " mr/36  addr : dump 36-bit value at addr\n"
                 " mr/f32 addr : dump float  value at addr\n"
                 " mr/vec addr : dump vector value at addr\n"
                 " c (or ^D)   : continue execution\n"
                 " h(elp)      : print this message\n\n";
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

void Debugger::cmd_list(Command& command) {
    isa::PrintVisitor printvis(std::cout);
    auto breakpc = cpu.pc.getCurrentPC();

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

void Debugger::cmd_flags(Command& command) {
    std::cout << "flags: " << cpu.f << "\n\n";
}

auto parse_mem_operand(Command& cmd) -> std::optional<uint64_t> {
    if (cmd.args.size() != 1) {
        fmt::print(std::cerr, "usage: {} [address]\n", cmd.name);
        return std::nullopt;
    }

    auto sAddr = std::string_view(cmd.args[0]);

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

    if (res.ec == std::errc{} && res.ptr == (sAddr.data() + sAddr.size())) {
        return val;
    } else {
        return std::nullopt;
    }
}

void Debugger::cmd_m32(Command& cmd) {
    if (auto addr = parse_mem_operand(cmd)) {
        uint32_t val = mem.read32(*addr);
        fmt::print("{:#011x} : {:#010x}\n", *addr, val);
    }
}

void Debugger::cmd_m36(Command& cmd) {
    if (auto addr = parse_mem_operand(cmd)) {
        uint32_t val = mem.read36(*addr);
        fmt::print("{:#011x} : {:#011x}\n", *addr, val);
    }
}

void Debugger::cmd_f32(Command& cmd) {
    if (auto addr = parse_mem_operand(cmd)) {
        uint32_t val = mem.read32(*addr);
        fmt::print("{:#011x} : {}\n", *addr, bit_cast<float>(val));
    }
}

void Debugger::cmd_vec(Command& cmd) {
    if (auto addr = parse_mem_operand(cmd)) {
        f32x4 vec = mem.readVec(*addr);
        fmt::print("{:#011x} : {}\n", *addr, vec);
    }
}
