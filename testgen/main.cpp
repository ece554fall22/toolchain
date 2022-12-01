#include <fstream>
#include <iostream>
#include <random>

#include <argparse/argparse.hpp>
#include <pcg_random.hpp>

#include <morph/encoder.h>
#include <morph/isa.h>
#include <morph/ty.h>

using isa::ScalarArithmeticOp;

template <typename RNG, typename T, size_t N>
auto choose(RNG& rng, std::array<T, N>& arr) -> const T& {
    return arr[rng(N)];
}
// template<typename RNG, typename E> auto choose(RNG& rng) -> E {
//     return E::count__;
// }
// template<typename RNG, size_t N> auto rand_bits(RNG& rng) -> bits<N> {
//     return bits<N>(rng(bits<N>::idx))
// }

template <typename RNG> auto randomAOp(RNG& rng) -> ScalarArithmeticOp {
    std::array<ScalarArithmeticOp, 8> aops = {
        ScalarArithmeticOp::Add, ScalarArithmeticOp::Sub,
        ScalarArithmeticOp::Mul, ScalarArithmeticOp::And,
        ScalarArithmeticOp::Or,  ScalarArithmeticOp::Xor,
        ScalarArithmeticOp::Shr, ScalarArithmeticOp::Shl};

    return choose(rng, aops);
}

template <typename RNG> auto randomAIOp(RNG& rng) -> ScalarArithmeticOp {
    std::array<ScalarArithmeticOp, 8> aiops = {
        ScalarArithmeticOp::Add, ScalarArithmeticOp::Sub,
        ScalarArithmeticOp::And, ScalarArithmeticOp::Or,
        ScalarArithmeticOp::Xor, ScalarArithmeticOp::Shr,
        ScalarArithmeticOp::Shl};

    return choose(rng, aiops);
}

template <typename RNG> auto randomRegIdx(RNG& rng) -> reg_idx {
    return reg_idx(rng(reg_idx::mask));
}

template <typename RNG> auto randomVRegIdx(RNG& rng) -> reg_idx {
    return vreg_idx(rng(vreg_idx::mask));
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser ap("testgen");

    ap.add_argument("-o", "--output")
        .help("output path")
        .metavar("OUT")
        .default_value(std::string{"test.o"});

    ap.add_argument("-s", "--seed")
        .help("seed for random gen")
        .metavar("SEED")
        .scan<'i', uint64_t>();

    try {
        ap.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << ap;
        std::exit(1);
    }

    pcg32 rng(ap.present<uint64_t>("-s").value_or(0));
    if (!ap.is_used("-s")) {
        // pcg_extras::seed_seq_from<std::random_device> seed_source;
        rng.seed(pcg_extras::seed_seq_from<std::random_device>());
    }

    for (size_t i = 0; i < 10; i++) {
        std::cout << randomRegIdx(rng) << '\n';
    }

    std::vector<uint32_t> output(16, 0);
    {
        std::ofstream fOut(ap.get<std::string>("--output"), std::ios::binary);
        for (uint32_t word : output) {
            char v[4];
            v[0] = (word >> 0) & 0xff;
            v[1] = (word >> 8) & 0xff;
            v[2] = (word >> 16) & 0xff;
            v[3] = (word >> 24) & 0xff;
            fOut.write(v, sizeof(v));
        }
    }
}
