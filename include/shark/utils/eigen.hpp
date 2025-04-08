#pragma once

#include <shark/types/u128.hpp>
#include <shark/types/span.hpp>

namespace shark {
    namespace matrix {
        
        template <typename T>
        using MatType = Eigen::Map<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;
        template <typename T>
        using MatTypeConst = Eigen::Map<const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>;

        template <typename T>
        inline MatType<T> getMat(u64 a, u64 b, shark::span<T> &X)
        {
            return MatType<T>(X.data(), a, b);
        }

        template <typename T>
        inline MatTypeConst<T> getMat(u64 a, u64 b, const shark::span<T> &X)
        {
            return MatTypeConst<T>(X.data(), a, b);
        }
    }
}