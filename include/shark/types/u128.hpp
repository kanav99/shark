#pragma once

#include <cstdint>
#include <Eigen/Dense>
#include <cryptoTools/Common/Defines.h>

#include <shark/types/u64.hpp>
#include <shark/types/u8.hpp>

namespace shark {
    typedef osuCrypto::block block;
    typedef __uint128_t u128;

    inline u64 getLow(const u128 &x)
    {
        return (u64)x;
    }

    inline void setLow(u128 &x, u64 low)
    {
        ((u64*)&x)[0] = low;
    }

    inline u64 getHigh(const u128 &x)
    {
        return ((u64*)&x)[1];
    }

    inline void setHigh(u128 &x, u64 high)
    {
        ((u64*)&x)[1] = high;
    }
}
