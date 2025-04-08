#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/utils/assert.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 n = 10;

    shark::span<u64> X(n * n);
    shark::span<u64> Y(n * n);

    for (u64 i = 0; i < n * n; ++i)
    {
        X[i] = i;
    }

    for (u64 i = 0; i < n * n; ++i)
    {
        Y[i] = n * n + i;
    }

    auto Z_expected = matmul::emul(n, n, n, X, Y);

    input::call(X, 0);
    input::call(Y, 1);
    auto Z = matmul::call(n, n, n, X, Y);
    output::call(Z);
    finalize::call();

    if (party != DEALER)
    {
        for (u64 i = 0; i < n * n; ++i)
        {
            // std::cout << Z[i] << " " << Z_expected[i] << std::endl;
            always_assert(Z[i] == Z_expected[i]);
        }
    }
}
