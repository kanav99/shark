#pragma once
#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>

namespace shark
{
    namespace protocols
    {
        namespace lut
        {
            void gen(const shark::span<u64> &r_X, shark::span<u64> &r_Y, const std::vector<u64> &lut, int bin);
            void eval(const shark::span<u64> &X, shark::span<u64> &Y, const std::vector<u64> &lut, int bin);
            void call(const shark::span<u64> &X, shark::span<u64> &Y, const std::vector<u64> &lut, int bin);
            shark::span<u64> call(const shark::span<u64> &X, const std::vector<u64> &lut, int bin);
        }

    }
}
