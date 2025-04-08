#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>

#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/relu.hpp>
#include <shark/protocols/ars.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/maxpool.hpp>
#include <shark/protocols/add.hpp>
#include <shark/protocols/sumpool.hpp>

#include <shark/utils/timer.hpp>
#include <shark/utils/assert.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

void fill(shark::span<u64> &X, u64 val)
{
    for (u64 i = 0; i < X.size(); i++)
        X[i] = val;
}

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 batch_size = 1;
    u64 image_size = 32 * 32 * 3;
    int f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(3 * 64 * 3 * 3);
    shark::span<u64> B1(64);
    shark::span<u64> F2(64 * 64 * 3 * 3);
    shark::span<u64> B2(64);
    shark::span<u64> F3(64 * 64 * 3 * 3);
    shark::span<u64> B3(64);
    shark::span<u64> F4(64 * 64 * 3 * 3);
    shark::span<u64> B4(64);
    shark::span<u64> F5(64 * 64 * 3 * 3);
    shark::span<u64> B5(64);
    shark::span<u64> F6(64 * 64 * 1 * 1);
    shark::span<u64> B6(64);
    shark::span<u64> F7(64 * 16 * 1 * 1);
    shark::span<u64> B7(64);
    shark::span<u64> W8(1024 * 10);
    shark::span<u64> B8(10);


    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        // f = 10;
        fill(F1, 1 << f);
        fill(B1, 1 << (2*f));
        fill(F2, 1 << f);
        fill(B2, 1 << (2*f));
        fill(F3, 1 << f);
        fill(B3, 1 << (2*f));
        fill(F4, 1 << f);
        fill(B4, 1 << (2*f));
        fill(F5, 1 << f);
        fill(B5, 1 << (2*f));
        fill(F6, 1 << f);
        fill(B6, 1 << (2*f));
        fill(F7, 1 << f);
        fill(B7, 1 << (2*f));
        fill(W8, 1 << f);
        fill(B8, 1 << (2*f));
        // f = 12;
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(F1, SERVER);
    input::call(B1, SERVER);
    input::call(F2, SERVER);
    input::call(B2, SERVER);
    input::call(F3, SERVER);
    input::call(B3, SERVER);
    input::call(F4, SERVER);
    input::call(B4, SERVER);
    input::call(F5, SERVER);
    input::call(B5, SERVER);
    input::call(F6, SERVER);
    input::call(B6, SERVER);
    input::call(F7, SERVER);
    input::call(B7, SERVER);
    input::call(W8, SERVER);
    input::call(B8, SERVER);
    shark::utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();
    shark::utils::start_timer("simc2");

    auto a1 = conv::call(3, 1, 1, 3, 64, 32, 32, Image, F1);
    auto a2 = add::call(a1, B1);
    auto a3 = relutruncate::call(a2, f);

    auto a4 = conv::call(3, 1, 1, 64, 64, 32, 32, a3, F2);
    auto a5 = add::call(a4, B2);
    auto a6 = relu::call(a5);
    auto a7 = sumpool::call(2, 0, 2, 64, 32, 32, a6);
    auto a8 = ars::call(a7, f + 2);

    auto a9 = conv::call(3, 1, 1, 64, 64, 16, 16, a8, F3);
    auto a10 = add::call(a9, B3);
    auto a11 = relutruncate::call(a10, f);

    auto a12 = conv::call(3, 1, 1, 64, 64, 16, 16, a11, F4);
    auto a13 = add::call(a12, B4);
    auto a14 = relu::call(a13);
    auto a15 = sumpool::call(2, 0, 2, 64, 16, 16, a14);
    auto a16 = ars::call(a15, f + 2);

    auto a17 = conv::call(3, 1, 1, 64, 64, 8, 8, a16, F5);
    auto a18 = add::call(a17, B5);
    auto a19 = relutruncate::call(a18, f);

    auto a20 = conv::call(1, 0, 1, 64, 64, 8, 8, a19, F6);
    auto a21 = add::call(a20, B6);
    auto a22 = relutruncate::call(a21, f);

    auto a23 = conv::call(1, 0, 1, 64, 16, 8, 8, a22, F7);
    auto a24 = add::call(a23, B7);
    auto a25 = relutruncate::call(a24, f);

    auto a26 = matmul::call(batch_size, 1024, 10, a25, W8);
    auto a27 = add::call(a26, B8);
    auto a28 = ars::call(a27, f);

    output::call(a28);

    shark::utils::stop_timer("simc2");

    finalize::call();

    shark::utils::print_all_timers();
}
