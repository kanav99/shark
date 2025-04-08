#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/maxpool.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 bs = 1250;
    shark::span<u64> X(bs * 30 * 30);

    input::call(X, 0);

    shark::utils::start_timer("maxpool");
    auto Y = maxpool::call(3, 0, 3, 1, 30, 30, X);
    shark::utils::stop_timer("maxpool");
    output::call(Y);
    
    finalize::call();
    shark::utils::print_all_timers();
}
