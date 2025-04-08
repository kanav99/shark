#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/sigmoid.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
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
    shark::utils::start_timer("sigmoid");
    auto Y = sigmoid::call(f, X);
    shark::utils::stop_timer("sigmoid");
    output::call(Y);
    finalize::call();
    if (party != DEALER)
    {
        for (int i = 0; i < n; ++i)
        {
            double x = double(i - (n/2)) / (double)(1ll << f);
            int64_t y = int64_t(::sigmoid(x) * (1ull << f));
            int64_t y_hat = Y[i];
            // LLAMA ensures ULP of 4
            always_assert(std::abs(y - y_hat) < 5);
        }
    }
}
