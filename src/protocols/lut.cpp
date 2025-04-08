#include <shark/protocols/lut.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        namespace lut
        {
            void gen(const shark::span<u64> &X, shark::span<u64> &Y, const std::vector<u64> &lut, int bin)
            {
                always_assert(X.size() == Y.size());
                always_assert(lut.size() == (1ull << bin));
                randomize(Y);

                shark::span<u64> Xneg(X.size());
                for (u64 i = 0; i < X.size(); i++)
                {
                    Xneg[i] = -X[i];
                }

                send_dpfring(Xneg, bin);
                send_authenticated_ashare(Y);
            }

            void eval(const shark::span<u64> &X, shark::span<u64> &Y, const std::vector<u64> &lut, int bin)
            {
                always_assert(X.size() == Y.size());
                always_assert(lut.size() == (1ull << bin));

                shark::utils::start_timer("key_read");
                auto dpfKeys = recv_dpfring(X.size(), bin);
                auto [R_share, R_tag] = recv_authenticated_ashare(X.size());
                shark::utils::stop_timer("key_read");

                shark::span<u128> Y_share(X.size());
                shark::span<u128> Y_tag(X.size());

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    auto x = X[i];

                    auto [res_share, res_tag] = crypto::dpfring_evalall_reduce(party, dpfKeys[i], lut, x);

                    Y_share[i] = res_share + R_share[i];
                    Y_tag[i] = res_tag + R_tag[i];
                }

                Y = authenticated_reconstruct(Y_share, Y_tag);

            }

            void call(const shark::span<u64> &X, shark::span<u64> &Y, const std::vector<u64> &lut, int bin)
            {
                if (party == DEALER)
                {
                    gen(X, Y, lut, bin);
                }
                else
                {
                    eval(X, Y, lut, bin);
                }
            }

            shark::span<u64> call(const shark::span<u64> &X, const std::vector<u64> &lut, int bin)
            {
                shark::span<u64> Y(X.size());
                call(X, Y, lut, bin);
                return Y;
            }
        }

    }
}
