#pragma once

class Unimplemented : public std::logic_error {
public:
    Unimplemented() : std::logic_error("unimplemented!") { };
};

void unimplemented() {
    throw Unimplemented();
}
