#pragma once

#include <optional>
#include <string>
#include <vector>

struct CPUState;
struct MemSystem;

struct Command {
    std::string name;
    std::vector<std::string> args;
};

class Debugger {
  public:
    Debugger(CPUState& cpu, MemSystem& mem)
        : cpu(cpu), mem(mem), enabled(false) {}

    void tick();
    void hitBreakpoint();

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

    bool enabled;
};