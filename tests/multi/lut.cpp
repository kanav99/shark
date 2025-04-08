#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/lut.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using u8 = shark::u8;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 n = 10000;
    u64 bin = 8;
    shark::span<u64> X(n);
    shark::span<u64> Y_expected(n);

    std::vector<u64> lut(1ull << bin);
    for (int i = 0; i < (1ull << bin); i++)
    {
        lut[i] = i + 1;
    }

    if (party == SERVER) {
        for (u64 i = 0; i < n; ++i)
        {
            X[i] = (rand()) % (1ull << bin);
            Y_expected[i] = lut[X[i]];
        }
    }

    input::call(X, 0);
    auto Y = lut::call(X, lut, bin);
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
