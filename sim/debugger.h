#pragma once

#include <csignal>
#include <optional>
#include <string>
#include <vector>

#include <morph/varint.h>

struct CPUState;
struct MemSystem;

struct Command {
    std::string name;
    std::vector<std::string> args;
};

class Debugger {
  public:
    Debugger(CPUState& cpu, MemSystem& mem, bool& quitting)
        : enabled(false), cpu(cpu), mem(mem), quitting(quitting) {}

    void tick();
    void hitBreakpoint(bits<25> signal);
    void simHaltedByUser();

    bool enabled;

  private:
    CPUState& cpu;
    MemSystem& mem;

    auto getcmd() -> std::optional<Command>;
    void dispatch(Command& cmd);

    void cmd_help(Command& cmd);
    void cmd_registers_scalar(Command& command);
    void cmd_registers_vector(Command& command);
    void cmd_registers_mat(Command& command);
    void cmd_list(Command& command);
    void cmd_flags(Command& command);
    void cmd_mr(Command& cmd);
    void cmd_mx(Command& cmd);

    bool& quitting;
};