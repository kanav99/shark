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
    
    u64 n = 64;
    shark::span<u64> X(n+2);
    shark::span<u64> X_clear(n+2);
    int f = 16;

    if (party == SERVER) {
        for (u64 i = 0; i < n; ++i)
        {
            X[i] = i + 1;
            X_clear[i] = i + 1;
        }
        X[n] = (1ull<< (2*f));
        X_clear[n] = (1ull<< (2*f));
        X[n+1] = (1ull<< (2*f)) + 100;
        X_clear[n+1] = (1ull<< (2*f)) + 100;
        
    }

    input::call(X, 0);
    // shark::utils::start_timer("reciprocal");
    auto Y = reciprocal::call(X, f);
    // shark::utils::stop_timer("reciprocal");
    output::call(Y);

    if (party == SERVER) {
        for (u64 i = 0; i < n + 2; ++i)
        {
            u64 expected = (1ull << (2*f)) / X_clear[i];
            std::cout << Y[i] << ", " << expected << ", " << std::abs(int64_t(Y[i] - expected)) << std::endl;
        }
    }
    finalize::call();
}
