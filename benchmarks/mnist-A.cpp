#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/matmul.hpp>
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

    u64 batch_size = 100;
    u64 image_size = 28 * 28;
    u64 n_inner = 128;
    u64 f = 12;

    shark::span<u64> Image(batch_size * image_size);
    shark::span<u64> W1(image_size * n_inner);
    shark::span<u64> B1(n_inner);
    shark::span<u64> W2(n_inner * n_inner);
    shark::span<u64> B2(n_inner);
    shark::span<u64> W3(n_inner * 10);
    shark::span<u64> B3(10);

    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        fill(W1, 1 << f);
        fill(B1, 1 << (2*f));
        fill(W2, 1 << f);
        fill(B2, 1 << (2*f));
        fill(W3, 1 << f);
        fill(B3, 1 << (2*f));
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(W1, SERVER);
    input::call(B1, SERVER);
    input::call(W2, SERVER);
    input::call(B2, SERVER);
    input::call(W3, SERVER);
    input::call(B3, SERVER);
    shark::utils::stop_timer("input");

    shark::utils::start_timer("mnist-A");

    auto a1 = matmul::call(batch_size, image_size, n_inner, Image, W1);
    auto a2 = add::call(a1, B1);
    auto a3 = relutruncate::call(a2, f);
    auto a4 = matmul::call(batch_size, n_inner, n_inner, a3, W2);
    auto a5 = add::call(a4, B2);
    auto a6 = relutruncate::call(a5, f);
    auto a7 = matmul::call(batch_size, n_inner, 10, a6, W3);
    auto a8 = add::call(a7, B3);
    auto a9 = relutruncate::call(a8, f);
    shark::utils::stop_timer("mnist-A");

    shark::utils::start_timer("output");
    output::call(a9);
    shark::utils::stop_timer("output");
    
    if (party != DEALER)
        std::cout << a9 << std::endl;

    finalize::call();
    shark::utils::print_all_timers();

}
