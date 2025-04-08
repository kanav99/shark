#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/reciprocal.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 n = 1000000;
    shark::span<u64> X(n);
    int f = 16;

    for (u64 i = 0; i < n; i++)
        X[i] = 0;

    if (party != DEALER)
        peer->sync();

    shark::utils::start_timer("reciprocal");
    auto Y = reciprocal::call(X, f);
    shark::utils::stop_timer("reciprocal");
    output::call(Y);
    
    finalize::call();
    shark::utils::print_all_timers();
}
