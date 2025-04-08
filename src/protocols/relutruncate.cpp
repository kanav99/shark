#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        namespace relutruncate
        {
            void gen(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                always_assert(X.size() == Y.size());
                randomize(Y);

                send_dcfbit(X, 64);
                send_dcfbit(X, f);

                shark::span<u8> r_d(X.size());
                shark::span<u8> r_w(X.size());
                shark::span<u8> r_t(X.size());

                randomize(r_d);
                randomize(r_w);
                randomize(r_t);

                send_authenticated_bshare(r_d);
                send_authenticated_bshare(r_w);
                send_authenticated_bshare(r_t);

                shark::span<u64> T(X.size() * 8);
                for (u64 i = 0; i < X.size(); ++i)
                {
                    for (u64 j = 0; j < 8; ++j)
                    {
                        u8 d = j % 2;
                        u8 w = (j / 2) % 2;
                        u8 t = (j / 4) % 2;

                        if (d ^ r_d[i])
                            T[i * 8 + j] = ((1ull << (64 - f)) * (w ^ r_w[i])) - (t ^ r_t[i]) - (X[i] >> f) + Y[i];
                        else
                            T[i * 8 + j] = Y[i];
                    }

                }
                send_authenticated_ashare(T);

                shark::span<u64> r_d_ext(X.size());
                for (u64 i = 0; i < X.size(); ++i)
                {
                    r_d_ext[i] = r_d[i];
                }
                send_authenticated_ashare(r_d_ext);
            }

            void eval(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                always_assert(X.size() == Y.size());

                shark::utils::start_timer("key_read");
                auto dcfkeysN = recv_dcfbit(X.size(), 64);
                auto dcfkeysF = recv_dcfbit(X.size(), f);
                auto r_d = recv_authenticated_bshare(X.size());
                auto r_w = recv_authenticated_bshare(X.size());
                auto r_t = recv_authenticated_bshare(X.size());
                auto [T, T_tag] = recv_authenticated_ashare(X.size() * 8);
                auto [r_d_ext, r_d_ext_tag] = recv_authenticated_ashare(X.size());
                shark::utils::stop_timer("key_read");

                shark::span<FKOS> d(X.size());
                shark::span<FKOS> w(X.size());
                shark::span<FKOS> t(X.size());
                shark::span<u128> Y_share(X.size());
                shark::span<u128> Y_tag(X.size());

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    auto x = X[i];
                    auto y = x + (1ull << 63);

                    w[i] = dcfbit_eval(dcfkeysN[i], x);
                    d[i] = dcfbit_eval(dcfkeysN[i], y);
                    t[i] = dcfbit_eval(dcfkeysF[i], x);

                    d[i] = xor_fkos(w[i], d[i]);
                    if (y >= (1ull << 63))
                        d[i] = not_fkos(d[i]);

                    w[i] = xor_fkos(w[i], r_w[i]);
                    d[i] = xor_fkos(d[i], r_d[i]);
                    t[i] = xor_fkos(t[i], r_t[i]);
                }

                auto d_cap = authenticated_reconstruct(d);
                auto w_cap = authenticated_reconstruct(w);
                auto t_cap = authenticated_reconstruct(t);

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); ++i)
                {
                    u128 d_ext_share = r_d_ext[i];
                    u128 d_ext_tag = r_d_ext_tag[i];

                    if (d_cap[i] == 1)
                    {
                        d_ext_share = u128(party) - d_ext_share;
                        d_ext_tag = ring_key - d_ext_tag;
                    }
                    
                    u64 idx = 4 * t_cap[i] + 2 * w_cap[i] + d_cap[i];
                    Y_share[i] = d_ext_share * (X[i] >> f) + T[i * 8 + idx];
                    Y_tag[i] = d_ext_tag * (X[i] >> f) + T_tag[i * 8 + idx];
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
