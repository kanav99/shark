#pragma once

#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>
#include <shark/protocols/lrs.hpp>

namespace shark
{
    namespace protocols
    {
        namespace ars
        {
            inline void call(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                if (party == DEALER)
                {
                    lrs::call(X, Y, f);
                }
                else
                {
                    shark::span<u64> X_temp(X.size());
                    for (int i = 0; i < X.size(); ++i)
                    {
                        X_temp[i] = X[i] + (1ull << 63);
                    }
                    lrs::call(X_temp, Y, f);
                    for (int i = 0; i < X.size(); ++i)
                    {
                        Y[i] -= (1ull << (63-f));
                    }
                }
            }

            inline shark::span<u64> call(const shark::span<u64> &X, int f)
            {
                shark::span<u64> Y(X.size());
                call(X, Y, f);
                return Y;
            }

            inline shark::span<u64> emul(const shark::span<u64> &X, int f)
            {
                shark::span<u64> Y(X.size());
                for (int i = 0; i < X.size(); ++i)
                {
                    Y[i] = int64_t(X[i]) >> f;
                }
                return Y;
            }
        }
    }
}
