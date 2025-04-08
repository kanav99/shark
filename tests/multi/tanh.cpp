#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/tanh.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

double tanh(double x)
{
    return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
}

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    int n = 10000;
    shark::span<u64> X(n);
    int f = 12;

    if (party == SERVER) {
        for (int i = 0; i < n; ++i)
        {
            X[i] = (i - (n/2));
        }
    }

    input::call(X, 0);
    shark::utils::start_timer("tanh");
    auto Y = tanh::call(f, X);
    shark::utils::stop_timer("tanh");
    output::call(Y);
    finalize::call();
    if (party != DEALER)
    {
        for (int i = 0; i < n; ++i)
        {
            double x = double(i - (n/2)) / (double)(1ll << f);
            int64_t y = int64_t(::tanh(x) * (1ull << f));
            int64_t y_hat = Y[i];
            // TODO: LLAMA ensures ULP of 4, but we observe 5, why?
            always_assert(std::abs(y - y_hat) < 6);
        }
    }
}
