#include <shark/protocols/common.hpp>
#include <shark/crypto/dpfring.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

#include <iostream>

using u128 = shark::u128;
using namespace shark::protocols;
using namespace shark::crypto;
using namespace shark;

void dpfring_small_exhaustive()
{
    prngGlobal.SetSeed(osuCrypto::toBlock(0xdeadbeef));
    ring_key = 0xdeadbeef;
    int bin = 8;

    for (int trial = 0; trial < 100; ++trial)
    {
        u64 idx = rand() % (1ull << bin);
        auto [key0, key1] = dpfring_gen(bin, idx);

        std::vector<u64> lut(1ull << bin);
        for (int i = 0; i < (1ull << bin); i++)
        {
            lut[i] = rand();
        }

        for (int i = 0; i < (1ull << bin); i++)
        {
            // u64 x = i;
            auto [out_ring_0, out_tag_0] = dpfring_evalall_reduce(0, key0, lut, i);
            auto [out_ring_1, out_tag_1] = dpfring_evalall_reduce(1, key1, lut, i);

            u128 out_ring = out_ring_0 + out_ring_1;
            u128 out_tag = out_tag_0 + out_tag_1;
            always_assert(out_ring == lut[(idx + i) % (1ull << bin)]);
            always_assert(out_tag == ring_key * lut[(idx + i) % (1ull << bin)]);
        }
    }
}


int main(int argc, char **argv)
{
    dpfring_small_exhaustive();
}