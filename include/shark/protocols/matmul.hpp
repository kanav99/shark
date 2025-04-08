#pragma once

#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>

namespace shark {
    namespace protocols {
        namespace matmul {
            void gen(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z);
            void eval(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z);
            void call(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z);
            shark::span<u64> call(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y);
            shark::span<u64> emul(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y);

        }
    }
}