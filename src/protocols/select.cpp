#include <shark/protocols/select.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        namespace select
        {
            void gen(const shark::span<u8> &r_s, const shark::span<u64> &r_X, shark::span<u64> &r_Y)
            {
                always_assert(r_X.size() == r_s.size());
                always_assert(r_Y.size() == r_s.size());
                randomize(r_Y);

                shark::span<u64> u(r_X.size());
                shark::span<u64> v(r_X.size());
                shark::span<u64> w(r_X.size());
                shark::span<u64> z(r_X.size());

                for (u64 i = 0; i < r_X.size(); ++i)
                {
                    u[i] = r_s[i];
                    v[i] = r_X[i];
                    w[i] = u[i] * r_X[i] + r_Y[i];
                    z[i] = u[i] * r_X[i] * 2;
                }

                send_authenticated_ashare(u);
                send_authenticated_ashare(v);
                send_authenticated_ashare(w);
                send_authenticated_ashare(z);
            }

            void eval(const shark::span<u8> &s, const shark::span<u64> &X, shark::span<u64> &Y)
            {
                always_assert(X.size() == s.size());
                always_assert(Y.size() == s.size());
                shark::span<u128> Y_share(Y.size());
                shark::span<u128> Y_tag(Y.size());

                shark::utils::start_timer("key_read");
                auto [u, u_tag] = recv_authenticated_ashare(X.size());
                auto [v, v_tag] = recv_authenticated_ashare(X.size());
                auto [w, w_tag] = recv_authenticated_ashare(X.size());
                auto [z, z_tag] = recv_authenticated_ashare(X.size());
                shark::utils::stop_timer("key_read");

                // #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    if (s[i] == 0)
                    {
                        Y_share[i] = u[i] * X[i] + w[i] - z[i];
                        Y_tag[i] = u_tag[i] * X[i] + w_tag[i] - z_tag[i];
                    }
                    else
                    {
                        Y_share[i] = w[i] + X[i] * party - u[i] * X[i] - v[i];
                        Y_tag[i] = ring_key * X[i] - u_tag[i] * X[i] - v_tag[i] + w_tag[i];
                    }
                }

                Y = authenticated_reconstruct(Y_share, Y_tag);
                
            }

            void call(const shark::span<u8> &s, const shark::span<u64> &X, shark::span<u64> &Y)
            {
                if (party == DEALER)
                {
                    gen(s, X, Y);
                }
                else
                {
                    eval(s, X, Y);
                }
            }

            shark::span<u64> call(const shark::span<u8> &s, const shark::span<u64> &X)
            {
                shark::span<u64> Y(X.size());
                call(s, X, Y);
                return Y;
            }
        }

    }
}
