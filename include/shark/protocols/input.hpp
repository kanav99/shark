#pragma once

#include <shark/types/u64.hpp>
#include <shark/types/u8.hpp>
#include <shark/types/span.hpp>

namespace shark
{
    namespace protocols
    {
        namespace input
        {
            void gen(shark::span<u64> &X, int owner);
            void eval(shark::span<u64> &X, int owner);
            void call(shark::span<u64> &X, int owner);

            void gen(shark::span<u8> &X, int owner);
            void eval(shark::span<u8> &X, int owner);
            void call(shark::span<u8> &X, int owner);
        }
    }
}
