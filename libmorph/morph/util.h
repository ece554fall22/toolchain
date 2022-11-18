#pragma once

#include <string>

class Unimplemented : public std::logic_error {
  public:
    Unimplemented() : std::logic_error("unimplemented!"){};
};
inline void unimplemented() { throw Unimplemented(); }

class Todo : public std::logic_error {
  public:
    Todo() : std::logic_error("todo!"){};
    Todo(const std::string& desc) : std::logic_error("todo!: " + desc){};
};
inline void todo() { throw Todo(); }
inline void todo(const std::string& desc) { throw Todo(desc); }

class Panic : public std::logic_error {
  public:
    Panic() : std::logic_error("panic!"){};
    Panic(const std::string& desc) : std::logic_error("panic!: {}" + desc){};
};
inline void panic() { throw Panic(); }
inline void panic(const std::string& desc) { throw Panic(desc); }

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
