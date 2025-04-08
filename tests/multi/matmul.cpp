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
    
    shark::span<u64> X(2 * 2);
    shark::span<u64> Y(2 * 2);

    if (party == SERVER) {
        X[0] = 1;
        X[1] = 2;
        X[2] = 3;
        X[3] = 4;
    }

    if (party == CLIENT) {
        Y[0] = 5;
        Y[1] = 6;
        Y[2] = 7;
        Y[3] = 8;
    }

    // 1 2   5 6   19 22
    // 3 4   7 8   43 50

    input::call(X, 0);
    input::call(Y, 1);
    auto Z = matmul::call(2, 2, 2, X, Y);
    output::call(Z);
    finalize::call();

    if (party != DEALER)
    {
        always_assert(Z[0] == 19);
        always_assert(Z[1] == 22);
        always_assert(Z[2] == 43);
        always_assert(Z[3] == 50);
    }
}
