#include <shark/protocols/lrs.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        namespace lrs
        {
            void gen(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                always_assert(X.size() == Y.size());
                randomize(Y);

                send_dcfbit(X, 64);
                send_dcfbit(X, f);

                shark::span<u8> r_w(X.size());
                shark::span<u8> r_t(X.size());

                randomize(r_w);
                randomize(r_t);

                send_authenticated_bshare(r_w);
                send_authenticated_bshare(r_t);

                shark::span<u64> T(X.size() * 4);
                for (u64 i = 0; i < X.size(); ++i)
                {
                    for (u64 j = 0; j < 4; ++j)
                    {
                        u8 w = (j / 1) % 2;
                        u8 t = (j / 2) % 2;

                        T[i * 4 + j] = ((1ull << (64 - f)) * (w ^ r_w[i])) - (t ^ r_t[i]) - (X[i] >> f) + Y[i];
                    }

                }
                send_authenticated_ashare(T);
            }

            void eval(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                always_assert(X.size() == Y.size());

                shark::utils::start_timer("key_read");
                auto dcfkeysN = recv_dcfbit(X.size(), 64);
                auto dcfkeysF = recv_dcfbit(X.size(), f);
                auto r_w = recv_authenticated_bshare(X.size());
                auto r_t = recv_authenticated_bshare(X.size());
                auto [T, T_tag] = recv_authenticated_ashare(X.size() * 4);
                shark::utils::stop_timer("key_read");

                shark::span<FKOS> w(X.size());
                shark::span<FKOS> t(X.size());
                shark::span<u128> Y_share(X.size());
                shark::span<u128> Y_tag(X.size());

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    auto x = X[i];

                    w[i] = dcfbit_eval(dcfkeysN[i], x);
                    t[i] = dcfbit_eval(dcfkeysF[i], x);

                    w[i] = xor_fkos(w[i], r_w[i]);
                    t[i] = xor_fkos(t[i], r_t[i]);
                }

                auto w_cap = authenticated_reconstruct(w);
                auto t_cap = authenticated_reconstruct(t);

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); ++i)
                {
                    
                    u64 idx = 2 * t_cap[i] + w_cap[i];
                    Y_share[i] = u128(party) * (X[i] >> f) + T[i * 4 + idx];
                    Y_tag[i] = ring_key * (X[i] >> f) + T_tag[i * 4 + idx];
                }

                Y = authenticated_reconstruct(Y_share, Y_tag);

            }

            void call(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                if (party == DEALER)
                {
                    gen(X, Y, f);
                }
                else
                {
                    eval(X, Y, f);
                }
            }

            shark::span<u64> call(const shark::span<u64> &X, int f)
            {
                shark::span<u64> Y(X.size());
                call(X, Y, f);
                return Y;
            }
        }

    }
}
