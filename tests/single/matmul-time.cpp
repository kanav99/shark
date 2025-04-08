#include <shark/types/u128.hpp>
#include <shark/utils/timer.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/utils/eigen.hpp>
#include <iostream>

using namespace shark::matrix;
using u64 = shark::u64;

template <typename T>
void benchmark()
{
    shark::u64 n = 1000;
    shark::span<T> X(n * n);
    shark::span<T> Y(n * n);
    shark::span<T> Z(n * n);

    for (u64 i = 0; i < (n * n); i++)
    {
        X[i] = rand();
        Y[i] = rand();
    }

    auto mat_X = getMat(n, n, X);
    auto mat_Y = getMat(n, n, Y);
    auto mat_Z = getMat(n, n, Z);

    std::string type = std::to_string(8*sizeof(T));
    shark::utils::start_timer("matmul-" + type);
    for (int i = 0; i < 10; i++)
    {
        mat_Z = mat_X * mat_Y;
    }
    shark::utils::stop_timer("matmul-" + type);
}

int main()
{
    benchmark<shark::u128>();
    benchmark<shark::u64>();

    shark::utils::print_all_timers();
}