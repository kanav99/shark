#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relutruncate.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 n = 10000;
    shark::span<u64> X(n);
    int f = 12;

    if (party == SERVER) {
        for (u64 i = 0; i < n; ++i)
        {
            X[i] = (i - 500) * (1ull << f);
        }
    }

    input::call(X, 0);
    shark::utils::start_timer("relutruncate");
    auto Y = relutruncate::call(X, f);
    shark::utils::stop_timer("relutruncate");
    output::call(Y);
    finalize::call();
    if (party != DEALER)
    {
        for (u64 i = 0; i < n; ++i)
        {
            always_assert(Y[i] == (i > 500) * (i - 500));
        }
    }
}
