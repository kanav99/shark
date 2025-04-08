#include <shark/protocols/drelu.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        namespace drelu
        {
            void gen(const shark::span<u64> &X, shark::span<u8> &r_d)
            {
                always_assert(X.size() == r_d.size());
                u64 bw = mpspdz_32bit_compaison ? 32 : 64;

                randomize(r_d);

                shark::span<u64> x1(X.size());
                for (u64 i = 0; i < X.size(); ++i)
                {
                    x1[i] = -X[i];
                }
                send_dcfbit(x1, bw - 1);

                shark::span<u8> r_t(X.size());
                for (u64 i = 0; i < X.size(); ++i)
                {
                    // xor with msb of x1
                    r_t[i] = r_d[i] ^ 1 ^ ((x1[i] >> (bw - 1)) & 1);
                }

                send_authenticated_bshare(r_t);
            }

            void eval(const shark::span<u64> &X, shark::span<u8> &d)
            {
                always_assert(X.size() == d.size());
                u64 bw = mpspdz_32bit_compaison ? 32 : 64;

                shark::utils::start_timer("key_read");
                auto dcfkeys = recv_dcfbit(X.size(), bw - 1);
                auto r_d = recv_authenticated_bshare(X.size());
                shark::utils::stop_timer("key_read");

                shark::span<FKOS> d_fkos(X.size());
                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    auto x = X[i];
                    auto y = - x - 1;

                    d_fkos[i] = shark::crypto::dcfbit_eval(dcfkeys[i], y);

                    d_fkos[i] = xor_fkos(d_fkos[i], r_d[i]);
                    if ((x >> (bw - 1)) & 1)
                        d_fkos[i] = not_fkos(d_fkos[i]);
                }

                auto d_cap = authenticated_reconstruct(d_fkos);
                d = d_cap;
            }

            void call(const shark::span<u64> &X, shark::span<u8> &Y)
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

            shark::span<u8> call(const shark::span<u64> &X)
            {
                shark::span<u8> Y(X.size());
                call(X, Y);
                return Y;
            }
        }

    }
}
