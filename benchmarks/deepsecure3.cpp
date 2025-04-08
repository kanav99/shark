#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/add.hpp>
#include <shark/protocols/ars.hpp>
#include <shark/protocols/tanh.hpp>
#include <shark/protocols/sigmoid.hpp>

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
    u64 image_size = 617;
    u64 f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> W1(image_size * 50);
    shark::span<u64> B1(50);

    shark::span<u64> W2(50 * 26);
    shark::span<u64> B2(26);

    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        fill(W1, 1 << f);
        fill(B1, 1 << (2*f));
        fill(W2, 1 << f);
        fill(B2, 1 << (2*f));
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(W1, SERVER);
    input::call(B1, SERVER);
    input::call(W2, SERVER);
    input::call(B2, SERVER);
    shark::utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();

    shark::utils::start_timer("deepsecure3");

    // shark::utils::start_timer("matmul1");
    auto a1 = matmul::call(batch_size, image_size, 50, Image, W1);
    auto a2 = add::call(a1, B1);
    // shark::utils::stop_timer("matmul1");
    // shark::utils::start_timer("ars1");
    auto a3 = ars::call(a2, f);
    // shark::utils::stop_timer("ars1");
    // shark::utils::start_timer("tanh1");
    auto act1 = tanh::call(f, a3);
    // shark::utils::stop_timer("tanh1");

    // shark::utils::start_timer("matmul2");
    auto a4 = matmul::call(batch_size, 50, 26, act1, W2);
    auto a5 = add::call(a4, B2);
    // shark::utils::stop_timer("matmul2");
    // shark::utils::start_timer("ars2");
    auto a6 = ars::call(a5, f);
    // shark::utils::stop_timer("ars2");


    // shark::utils::start_timer("output");
    output::call(a6);
    // shark::utils::stop_timer("output");
    shark::utils::stop_timer("deepsecure3");
    
    // if (party != DEALER)
    //     std::cout << a6 << std::endl;

    finalize::call();
    shark::utils::print_all_timers();

}
