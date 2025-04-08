#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/maxpool.hpp>
#include <shark/utils/assert.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 bs = 100;
    u64 h = 3;
    u64 w = 3;
    u64 ci = 64;
    u64 f = 2;
    u64 padding = 0;
    u64 stride = 1;
    u64 outH = (h + 2 * padding - f) / stride + 1;
    u64 outW = (w + 2 * padding - f) / stride + 1;

    shark::span<u64> Img(bs * h * w * ci);

    for (u64 i = 0; i < bs; ++i)
    {
        for (u64 l = 0; l < ci; ++l)
        {
            for (u64 j = 0; j < h; ++j)
            {
                for (u64 k = 0; k < w; ++k)
                {
                    Img[i * h * w * ci + j * w * ci + k * ci + l] = j * w + k + 1;
                    // std::cout << Img[i * h * w * ci + j * w * ci + k * ci + l] << " ";
                }
            }

            // std::cout << std::endl;
        }
    }

    input::call(Img, 1);
    auto Z = maxpool::call(f, padding, stride, ci, h, w, Img);
    always_assert(Z.size() == bs * outH * outW * ci);
    output::call(Z);
    finalize::call();

    if (party != DEALER)
    {
        for (u64 i = 0; i < bs; ++i)
        {
            for (u64 l = 0; l < ci; ++l)
            {
                // std::cout << Z[i * outH * outW * ci + 0 * outW * ci + 0 * ci + l] << std::endl;
                // std::cout << Z[i * outH * outW * ci + 0 * outW * ci + 1 * ci + l] << std::endl;
                // std::cout << Z[i * outH * outW * ci + 1 * outW * ci + 0 * ci + l] << std::endl;
                // std::cout << Z[i * outH * outW * ci + 1 * outW * ci + 1 * ci + l] << std::endl;

                always_assert(Z[i * outH * outW * ci + 0 * outW * ci + 0 * ci + l] == 5);
                always_assert(Z[i * outH * outW * ci + 0 * outW * ci + 1 * ci + l] == 6);
                always_assert(Z[i * outH * outW * ci + 1 * outW * ci + 0 * ci + l] == 8);
                always_assert(Z[i * outH * outW * ci + 1 * outW * ci + 1 * ci + l] == 9);
            }
        }
    }
}
