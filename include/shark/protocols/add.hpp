#pragma once

#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>

namespace shark
{
    namespace protocols
    {
        namespace add
        {
            inline void _add(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                always_assert(X.size() == Z.size());
                always_assert(X.size() % Y.size() == 0);

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    Z[i] = X[i] + Y[i % Y.size()];
                }
            }

            inline void gen(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                _add(X, Y, Z);
            }

            inline void eval(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                _add(X, Y, Z);
            
            }

            inline void call(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                _add(X, Y, Z);
            }

            inline shark::span<u64> call(const shark::span<u64> &X, const shark::span<u64> &Y)
            {
                shark::span<u64> Z(X.size());
                _add(X, Y, Z);
                return Z;
            }
        }
    }
}
