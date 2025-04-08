#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 n = 1000;
    shark::span<u64> X(n * n);
    shark::span<u64> Y(n * n);

    input::call(X, 0);
    input::call(Y, 1);

    shark::utils::start_timer("matmul");
    auto Z = matmul::call(n, n, n, X, Y);
    shark::utils::stop_timer("matmul");
    output::call(Z);
    
    finalize::call();
    shark::utils::print_all_timers();
}
