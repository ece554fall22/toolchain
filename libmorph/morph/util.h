#pragma once

#include <string>

/// --- "we have Rust at home"
class Unimplemented : public std::logic_error {
  public:
    Unimplemented(const char* file, size_t line) : std::logic_error(fmt::format("unimplemented! at {}:{}", file, line)) {};
    Unimplemented(const char* file, size_t line, const std::string& desc) : std::logic_error(fmt::format("unimplemented! at {}:{}: {}", file, line, desc)) {};
};

class Todo : public std::logic_error {
  public:
    Todo(const char* file, size_t line) : std::logic_error(fmt::format("todo! at {}:{}", file, line)) {};
    Todo(const char* file, size_t line, const std::string& desc) : std::logic_error(fmt::format("todo! at {}:{}: {}", file, line, desc)) {};
};

class Panic : public std::logic_error {
  public:
    Panic(const char* file, size_t line) : std::logic_error(fmt::format("panic! at {}:{}", file, line)) {};
    Panic(const char* file, size_t line, const std::string& desc) : std::logic_error(fmt::format("panic! at {}:{}: {}", file, line, desc)) {};
};

#define panic(...) throw Panic(__FILE__, __LINE__, ##__VA_ARGS__)
#define todo(...) throw Panic(__FILE__, __LINE__, ##__VA_ARGS__)
#define unimplemented(...) throw Panic(__FILE__, __LINE__, ##__VA_ARGS__)

/// --- overload helpers for std::visit
template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
