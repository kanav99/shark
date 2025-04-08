#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>

#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/maxpool.hpp>
#include <shark/protocols/add.hpp>

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
    u64 image_size = 28 * 28;
    int f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(1 * 16 * 5 * 5);
    shark::span<u64> B1(16);
    shark::span<u64> F2(16 * 16 * 5 * 5);
    shark::span<u64> B2(16);
    shark::span<u64> W3(256 * 100);
    shark::span<u64> B3(100);
    shark::span<u64> W4(100 * 10);
    shark::span<u64> B4(10);

    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        fill(F1, 1 << f);
        fill(B1, 1 << (2*f));
        fill(F2, 1 << f);
        fill(B2, 1 << (2*f));
        fill(W3, 1 << f);
        fill(B3, 1 << (2*f));
        fill(W4, 1 << f);
        fill(B4, 1 << (2*f));
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(F1, SERVER);
    input::call(B1, SERVER);
    input::call(F2, SERVER);
    input::call(B2, SERVER);
    input::call(W3, SERVER);
    input::call(B3, SERVER);
    input::call(W4, SERVER);
    input::call(B4, SERVER);
    shark::utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();
    shark::utils::start_timer("simc1");

    // shark::utils::start_timer("100");
    auto a1 = conv::call(5, 0, 1, 1, 16, 28, 28, Image, F1);
    auto a2 = add::call(a1, B1);
    // shark::utils::stop_timer("100");
    // shark::utils::start_timer("101");
    auto a3 = maxpool::call(2, 0, 2, 16, 24, 24, a2);
    // shark::utils::stop_timer("101");
    // shark::utils::start_timer("102");
    auto a4 = relutruncate::call(a3, f);
    // shark::utils::stop_timer("102");

    // shark::utils::start_timer("103");
    auto a5 = conv::call(5, 0, 1, 16, 16, 12, 12, a4, F2);
    auto a6 = add::call(a5, B2);
    // shark::utils::stop_timer("103");
    // shark::utils::start_timer("104");
    auto a7 = maxpool::call(2, 0, 2, 16, 8, 8, a6);
    // shark::utils::stop_timer("104");
    // shark::utils::start_timer("105");
    auto a8 = relutruncate::call(a7, f);
    // shark::utils::stop_timer("105");

    // shark::utils::start_timer("106");
    auto a9 = matmul::call(batch_size, 256, 100, a8, W3);
    auto a10 = add::call(a9, B3);
    // shark::utils::stop_timer("106");
    // shark::utils::start_timer("107");
    auto a11 = relutruncate::call(a10, f);
    // shark::utils::stop_timer("107");

    // shark::utils::start_timer("108");
    auto a12 = matmul::call(batch_size, 100, 10, a11, W4);
    auto a13 = add::call(a12, B4);
    // shark::utils::stop_timer("108");
    // shark::utils::start_timer("109");
    auto a14 = relutruncate::call(a13, f);
    // shark::utils::stop_timer("109");

    output::call(a14);

    shark::utils::stop_timer("simc1");

    finalize::call();

    shark::utils::print_all_timers();
}
