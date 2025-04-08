#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relu.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 n = 1000000;
    shark::span<u64> X(n);

    for (u64 i = 0; i < n; i++)
        X[i] = 0;

    if (party != DEALER)
        peer->sync();

    shark::utils::start_timer("relu");
    auto Y = relu::call(X);
    shark::utils::stop_timer("relu");
    output::call(Y);
    
    finalize::call();
    shark::utils::print_all_timers();
}
