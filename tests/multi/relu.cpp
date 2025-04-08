#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relu.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using u8 = shark::u8;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 n = 10000;
    shark::span<u64> X(n);
    shark::span<u64> Y_expected(n);

    if (party == SERVER) {
        for (u64 i = 0; i < n; ++i)
        {
            X[i] = rand();
            auto d = rand() % 2;
            if (d == 0) {
                X[i] = -X[i];
                Y_expected[i] = 0;
            }
            else
            {
                Y_expected[i] = X[i];
            }
        }
    }

    input::call(X, 0);
    auto Y = relu::call(X);
    output::call(Y);
    finalize::call();
    if (party == SERVER)
    {
        for (u64 i = 0; i < n; ++i)
        {
            always_assert(Y[i] == Y_expected[i]);
        }
    }
}
