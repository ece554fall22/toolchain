#include <iostream>

#include "sema.h"

int main(int argc, char* argv[]) {
    SemanticsPass::dumpSemanticsTable(std::cout);

    return 0;
}