#pragma once
#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>

namespace shark
{
    namespace protocols
    {
        namespace relu
        {
            void gen(const shark::span<u64> &r_X, shark::span<u64> &r_Y);
            void eval(const shark::span<u64> &X, shark::span<u64> &Y);
            void call(const shark::span<u64> &X, shark::span<u64> &Y);
            shark::span<u64> call(const shark::span<u64> &X);
        }

    }
}
