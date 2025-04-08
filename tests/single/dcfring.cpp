#include <shark/protocols/common.hpp>
#include <shark/crypto/dcfring.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

#include <iostream>

using u128 = shark::u128;
using namespace shark::protocols;
using namespace shark::crypto;
using namespace shark;

void dcfring_small_exhaustive()
{
    prngGlobal.SetSeed(osuCrypto::toBlock(0xdeadbeef));
    ring_key = 0xdeadbeef;
    int bin = 8;

    for (int trial = 0; trial < 100; ++trial)
    {
        u64 idx = rand() % (1ull << bin);
        auto [key0, key1] = dcfring_gen(bin, idx);

        for (int i = 0; i < (1ull << bin); i++)
        {
            u64 x = i;
            auto [out_ring_0, out_tag_0] = dcfring_eval(0, key0, x);
            auto [out_ring_1, out_tag_1] = dcfring_eval(1, key1, x);

            u128 out_ring = out_ring_0 + out_ring_1;
            u128 out_tag = out_tag_0 + out_tag_1;
            if (x < idx)
            {
                always_assert(out_ring == 1);
                always_assert(out_tag == ring_key);
            }
            else
            {
                always_assert(out_ring == 0);
                always_assert(out_tag == 0);
            }
        }
    }
}

void dcfring_large_random()
{
    prngGlobal.SetSeed(osuCrypto::toBlock(0xdeadbeef));
    ring_key = 0xdeadbeef;
    int bin = 64;

    shark::utils::start_timer("random-test");
    for (int trial = 0; trial < 100; ++trial)
    {
        u64 idx = rand();
        idx = (idx << 32) | rand();
        auto [key0, key1] = dcfring_gen(bin, idx);

        for (int i = 0; i < 5000; i++)
        {
            u64 x = rand();
            x = (x << 32) | rand();
            auto [out_0, out_tag_0] = dcfring_eval(0, key0, x);
            auto [out_1, out_tag_1] = dcfring_eval(1, key1, x);

            u128 out = out_0 + out_1;
            u128 out_tag = out_tag_0 + out_tag_1;
            if (x < idx)
            {
                always_assert(out == 1);
                always_assert(out_tag == ring_key);
            }
            else
            {
                always_assert(out == 0);
                always_assert(out_tag == 0);
            }
        }
    }
    shark::utils::stop_timer("random-test");
    shark::utils::print_timer("random-test");
}


int main(int argc, char **argv)
{
    dcfring_small_exhaustive();
    dcfring_large_random();
}