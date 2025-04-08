#pragma once

#include <shark/types/u64.hpp>
#include <shark/types/span.hpp>

namespace shark {
    namespace protocols {
        namespace conv {
            void  gen(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter, shark::span<u64> &Z);
            void eval(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter, shark::span<u64> &Z);
            void call(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter, shark::span<u64> &Z);
            shark::span<u64> call(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter);
            shark::span<u64> emul(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter);

        }
    }
}