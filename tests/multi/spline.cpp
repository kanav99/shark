#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/spline.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    u64 n = 16;
    shark::span<u64> X(n);

    if (party == SERVER) {
        for (u64 i = 0; i < n; i++)
        {
            X[i] = i;
        }
    }

    input::call(X, 0);

    std::vector<u64> knots = {3, 8, 13};
    u64 bin = 4;
    u64 degree = 1;
    std::vector<u64> polynomials = {1, 3, 2, 5, 3, 7, 4, 9};

    auto Y = spline::call(bin, degree, knots, polynomials, X);
    output::call(Y);

    if (party != DEALER) 
    {
        u64 currKnot = 0;
        u64 i = 0;
        for (u64 currKnot = 0; currKnot < knots.size() + 1; ++currKnot)
        {
            if (currKnot == knots.size())
            {
                while(i < n)
                {
                    u64 expected_y = 0;
                    for (u64 j = 0; j < degree + 1; ++j)
                    {
                        expected_y += polynomials[2 * currKnot + j] * pow(i, j);
                    }
                    always_assert(Y[i] == expected_y);
                    ++i;
                }
                break;
            }
            else
            {
                while(i < knots[currKnot])
                {
                    u64 expected_y = 0;
                    for (u64 j = 0; j < degree + 1; ++j)
                    {
                        expected_y += polynomials[2 * currKnot + j] * pow(i, j);
                    }
                    always_assert(Y[i] == expected_y);
                    ++i;
                }
            }
        }
        
    }

    finalize::call();
}

/*
0 3 8 13 16
 1 2 3  4

0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
1 1 1 2 2 2 2 2 3 3 3  3  3  4  4  4

0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
4 4 4 1 1 1 2 2 2 2 2  3  3  3  3  3  
*/