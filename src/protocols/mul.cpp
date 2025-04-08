#include <shark/protocols/matmul.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

namespace shark {
    namespace protocols {
        namespace mul {

            void gen(const shark::span<u64> &r_X, const shark::span<u64> &r_Y, shark::span<u64> &r_Z)
            {
                u64 n = r_X.size();
                always_assert(r_Y.size() == n);
                always_assert(r_Z.size() == n);

                randomize(r_Z);

                shark::span<u64> r_C(n);
                // r_C = r_X @ r_Y + r_Z
                #pragma omp parallel for
                for (u64 i = 0; i < n; i++)
                {
                    r_C[i] = r_X[i] * r_Y[i] + r_Z[i];
                }

                send_authenticated_ashare(r_X);
                send_authenticated_ashare(r_Y);
                send_authenticated_ashare(r_C);
            }

            void eval(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                u64 n = X.size();
                always_assert(Y.size() == n);
                always_assert(Z.size() == n);

                shark::utils::start_timer("key_read");
                auto [r_X, r_X_tag] = recv_authenticated_ashare(n);
                auto [r_Y, r_Y_tag] = recv_authenticated_ashare(n);
                auto [r_Z, r_Z_tag] = recv_authenticated_ashare(n);
                shark::utils::stop_timer("key_read");


                shark::span<u128> Z_share(n);
                shark::span<u128> Z_tag(n);

                // Z = r_Z + X @ Y - r_X @ Y - X @ r_Y
                #pragma omp parallel for
                for (u64 i = 0; i < n; i++)
                {
                    Z_share[i] = r_Z[i] + (u128(party) * X[i] - r_X[i]) * Y[i] - r_Y[i] * X[i];
                    Z_tag[i] = r_Z_tag[i] + (ring_key * X[i] - r_X_tag[i]) * Y[i] - r_Y_tag[i] * X[i];
                }

                Z = authenticated_reconstruct(Z_share, Z_tag);
            }

            void call(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                if (party == DEALER)
                {
                    gen(X, Y, Z);
                }
                else
                {
                    eval(X, Y, Z);
                }
            }

            shark::span<u64> call(const shark::span<u64> &X, const shark::span<u64> &Y)
            {
                shark::span<u64> Z(X.size());
                call(X, Y, Z);
                return Z;
            }
        }
    }
}