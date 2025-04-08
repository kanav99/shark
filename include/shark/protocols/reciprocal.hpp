#pragma once

#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>

namespace shark
{
    namespace protocols
    {
        namespace reciprocal
        {
            void gen(const shark::span<u64> &X, shark::span<u64> &Y, int f);
            void eval(const shark::span<u64> &X, shark::span<u64> &Y, int f);
            void call(const shark::span<u64> &X, shark::span<u64> &Y, int f);
            shark::span<u64> call(const shark::span<u64> &X, int f);
        }
    }
}
