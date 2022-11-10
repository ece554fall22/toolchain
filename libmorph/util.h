#pragma once

class Unimplemented : public std::logic_error {
public:
    Unimplemented() : std::logic_error("unimplemented!") { };
};

inline void unimplemented() {
    throw Unimplemented();
}
