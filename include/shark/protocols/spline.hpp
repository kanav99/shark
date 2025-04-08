#pragma once

#include <shark/types/span.hpp>
#include <vector>

namespace shark
{
    namespace protocols
    {
        namespace spline
        {
            void gen(int n, int degree, const shark::span<u64> &X, shark::span<u64> &Y);

            void eval(int n, int degree, const std::vector<u64> &knots, const std::vector<u64> &polynomials, const shark::span<u64> &X, shark::span<u64> &Y);

            void call(int bin, int degree, const std::vector<u64> &knots, const std::vector<u64> &polynomials, const shark::span<u64> &X, shark::span<u64> &Y);

            shark::span<u64> call(int bin, int degree, const std::vector<u64> &knots, const std::vector<u64> &polynomials, const shark::span<u64> &X);
        }
    }
}