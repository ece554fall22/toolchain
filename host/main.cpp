#include <argparse/argparse.hpp>
#include <fmt/core.h>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("host");

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    return 0;
}
