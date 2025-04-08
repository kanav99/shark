#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/add.hpp>
#include <shark/protocols/ars.hpp>

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
    u64 f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(5 * 5 * 1 * 5);
    shark::span<u64> B1(5);

    shark::span<u64> W2(845 * 100);
    shark::span<u64> B2(100);

    shark::span<u64> W3(100 * 10);
    shark::span<u64> B3(10);

    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        fill(F1, 1 << f);
        fill(B1, 1 << (2*f));
        fill(W2, 1 << f);
        fill(B2, 1 << (2*f));
        fill(W3, 1 << f);
        fill(B3, 1 << (2*f));
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(F1, SERVER);
    input::call(B1, SERVER);
    input::call(W2, SERVER);
    input::call(B2, SERVER);
    input::call(W3, SERVER);
    input::call(B3, SERVER);
    shark::utils::stop_timer("input");

    shark::utils::start_timer("deepsecure1");

    // shark::utils::start_timer("conv1");
    auto a1 = conv::call(5, 1, 2, 1, 5, 28, 28, Image, F1);
    auto a2 = add::call(a1, B1);
    // shark::utils::stop_timer("conv1");
    // shark::utils::start_timer("rt1");
    auto a3 = relutruncate::call(a2, f);
    // shark::utils::stop_timer("rt1");

    // shark::utils::start_timer("matmul1");
    auto a4 = matmul::call(batch_size, 845, 100, a3, W2);
    auto a5 = add::call(a4, B2);
    // shark::utils::stop_timer("matmul1");
    // shark::utils::start_timer("rt2");
    auto a6 = relutruncate::call(a5, f);
    // shark::utils::stop_timer("rt2");

    // shark::utils::start_timer("matmul2");
    auto a7 = matmul::call(batch_size, 100, 10, a6, W3);
    auto a8 = add::call(a7, B3);
    // shark::utils::stop_timer("matmul2");
    // shark::utils::start_timer("ars3");
    auto a9 = ars::call(a8, f);
    // shark::utils::stop_timer("ars3");

    // shark::utils::start_timer("output");
    output::call(a9);
    // shark::utils::stop_timer("output");
    shark::utils::stop_timer("deepsecure1");
    
    // if (party != DEALER)
    //     std::cout << a9 << std::endl;

    finalize::call();
    shark::utils::print_all_timers();

}
