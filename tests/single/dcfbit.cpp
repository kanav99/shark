#include <shark/protocols/common.hpp>
#include <shark/crypto/dcfbit.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

#include <iostream>

using namespace shark::protocols;
using namespace shark::crypto;
using namespace shark;

void dcfbit_small_exhaustive()
{
    prngGlobal.SetSeed(osuCrypto::toBlock(0xdeadbeef));
    bit_key = 0xdeadbeef;
    int bin = 8;

    for (int trial = 0; trial < 100; ++trial)
    {
        u64 idx = rand() % (1ull << bin);
        auto [key0, key1] = dcfbit_gen(bin, idx);

        for (int i = 0; i < (1ull << bin); i++)
        {
            u64 x = i;
            auto [out_bit_0, out_tag_1_0] = dcfbit_eval(key0, x);
            auto [out_bit_1, out_tag_1_1] = dcfbit_eval(key1, x);

            bool out_bit = out_bit_0 ^ out_bit_1;
            u64 out_tag_1 = out_tag_1_0 ^ out_tag_1_1;
            if (x < idx)
            {
                always_assert(out_bit == 1);
                always_assert(out_tag_1 == bit_key);
            }
            else
            {
                always_assert(out_bit == 0);
                always_assert(out_tag_1 == 0);
            }
        }
    }
}

void dcfbit_large_random()
{
    prngGlobal.SetSeed(osuCrypto::toBlock(0xdeadbeef));
    bit_key = 0xdeadbeef;
    int bin = 64;

    shark::utils::start_timer("random-test");
    for (int trial = 0; trial < 100; ++trial)
    {
        u64 idx = rand();
        idx = (idx << 32) | rand();
        auto [key0, key1] = dcfbit_gen(bin, idx);

        for (int i = 0; i < 5000; i++)
        {
            u64 x = rand();
            x = (x << 32) | rand();
            auto [out_bit_0, out_tag_1_0] = dcfbit_eval(key0, x);
            auto [out_bit_1, out_tag_1_1] = dcfbit_eval(key1, x);

            bool out_bit = out_bit_0 ^ out_bit_1;
            u64 out_tag_1 = out_tag_1_0 ^ out_tag_1_1;
            if (x < idx)
            {
                always_assert(out_bit == 1);
                always_assert(out_tag_1 == bit_key);
            }
            else
            {
                always_assert(out_bit == 0);
                always_assert(out_tag_1 == 0);
            }
        }
    }
    shark::utils::stop_timer("random-test");
    shark::utils::print_timer("random-test");
}

int main(int argc, char **argv)
{
    dcfbit_small_exhaustive();
    dcfbit_large_random();
}