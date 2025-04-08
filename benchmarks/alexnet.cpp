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
    u64 image_size = 224 * 224 * 3;
    int f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(3 * 96 * 11 * 11);
    shark::span<u64> F2(96 * 256 * 5 * 5);
    shark::span<u64> F3(256 * 384 * 3 * 3);
    shark::span<u64> F4(384 * 384 * 3 * 3);
    shark::span<u64> F5(384 * 256 * 3 * 3);
    shark::span<u64> W6(256 * 6 * 6 * 9216);
    shark::span<u64> W7(9216 * 4096);
    shark::span<u64> W8(4096 * 1000);


    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        fill(F1, 1 << f);
        fill(F2, 1 << f);
        fill(F3, 1 << f);
        fill(F4, 1 << f);
        fill(F5, 1 << f);
        fill(W6, 1 << f);
        fill(W7, 1 << f);
        fill(W8, 1 << f);
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(F1, SERVER);
    input::call(F2, SERVER);
    input::call(F3, SERVER);
    input::call(F4, SERVER);
    input::call(F5, SERVER);
    input::call(W6, SERVER);
    input::call(W7, SERVER);
    input::call(W8, SERVER);
    shark::utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();
    shark::utils::start_timer("alexnet");

    auto a1 = conv::call(11, 2, 4, 3, 96, 224, 224, Image, F1);
    auto a2 = relu::call(a1);
    auto a3 = sumpool::call(3, 0, 2, 96, 55, 55, a2);
    // std::cout << "yo\n";
    auto a4 = ars::call(a3, f + 3); // fix: div by 9, not 8

    auto a5 = conv::call(5, 2, 1, 96, 256, 27, 27, a4, F2);
    auto a6 = relu::call(a5);
    auto a7 = sumpool::call(3, 0, 2, 256, 27, 27, a6);
    // std::cout << "yo\n";
    auto a8 = ars::call(a7, f + 3); // fix: div by 9, not 8

    auto a9 = conv::call(3, 1, 1, 256, 384, 13, 13, a8, F3);
    auto a10 = relutruncate::call(a9, f);

    auto a11 = conv::call(3, 1, 1, 384, 384, 13, 13, a10, F4);
    auto a12 = relutruncate::call(a11, f);

    auto a13 = conv::call(3, 1, 1, 384, 256, 13, 13, a12, F5);
    auto a14 = relu::call(a13);
    auto a15 = sumpool::call(3, 0, 2, 256, 13, 13, a14);
    // std::cout << "yo\n";
    auto a16 = ars::call(a15, f + 3); // fix: div by 9, not 8

    auto a17 = matmul::call(batch_size, 256 * 6 * 6, 9216, a16, W6);
    auto a18 = relutruncate::call(a17, f);

    auto a19 = matmul::call(batch_size, 9216, 4096, a18, W7);
    auto a20 = relutruncate::call(a19, f);

    auto a21 = matmul::call(batch_size, 4096, 1000, a20, W8);
    auto a22 = ars::call(a21, f);

    output::call(a22);

    shark::utils::stop_timer("alexnet");

    finalize::call();

    shark::utils::print_all_timers();
}


// conv avgpool conv avgpool conv conv conv avgpool fc fc fc
