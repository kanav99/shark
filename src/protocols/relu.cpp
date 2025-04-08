#include <shark/protocols/relu.hpp>
#include <shark/protocols/drelu.hpp>
#include <shark/protocols/select.hpp>
#include <shark/protocols/common.hpp>

namespace shark
{
    namespace protocols
    {
        namespace relu
        {
            void gen(const shark::span<u64> &r_X, shark::span<u64> &r_Y)
            {
                auto d = drelu::call(r_X);
                select::call(d, r_X, r_Y);
            }

            void eval(const shark::span<u64> &X, shark::span<u64> &Y)
            {
                auto d = drelu::call(X);
                select::call(d, X, Y);
            }

            void call(const shark::span<u64> &X, shark::span<u64> &Y)
            {
                if (party == DEALER)
                {
                    gen(X, Y);
                }
                else
                {
                    eval(X, Y);
                }
            }

            shark::span<u64> call(const shark::span<u64> &X)
            {
                shark::span<u64> Y(X.size());
                call(X, Y);
                return Y;
            }
        }

    }
}
