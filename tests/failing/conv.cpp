#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/utils/assert.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 bs = 5; // bs = 1 works
    u64 h = 28;
    u64 w = 28;
    u64 ci = 3;
    u64 co = 64;
    u64 f = 3;
    u64 padding = 0;
    u64 stride = 1;
    u64 outH = (h + 2 * padding - f) / stride + 1;
    u64 outW = (w + 2 * padding - f) / stride + 1;

    osuCrypto::PRNG initPRNG(osuCrypto::toBlock(0xdeadbeef));

    shark::span<u64> Filter(co * f * f * ci);
    shark::span<u64> Img(bs * h * w * ci);

    for (u64 i = 0; i < co * f * f * ci; ++i)
    {
        // Filter[i] = 1;
        Filter[i] = initPRNG.get<u64>();
    }

    for (u64 i = 0; i < bs * h * w * ci; ++i)
    {
        // Img[i] = 1;
        Img[i] = initPRNG.get<u64>();
    }

    auto Z_expected = conv::emul(f, padding, stride, ci, co, h, w, Img, Filter);

    input::call(Filter, 0);
    input::call(Img, 1);
    auto Z = conv::call(f, padding, stride, ci, co, h, w, Img, Filter);
    always_assert(Z.size() == bs * outH * outW * co);
    output::call(Z);
    finalize::call();

    if (party != DEALER)
    {
        for (u64 i = 0; i < Z.size(); ++i)
        {
            // std::cout << Z[i] << std::endl;
            always_assert(Z[i] == Z_expected[i]);
            // always_assert(Z[i] == f * f * ci);
        }
    }
}
