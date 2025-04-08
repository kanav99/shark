#include <shark/protocols/matmul.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>
#include <shark/utils/eigen.hpp>

using namespace shark::matrix;

namespace shark {
    namespace protocols {
        namespace matmul {

            void gen(u64 a, u64 b, u64 c, const shark::span<u64> &r_X, const shark::span<u64> &r_Y, shark::span<u64> &r_Z)
            {
                always_assert(r_X.size() == a * b);
                always_assert(r_Y.size() == b * c);
                always_assert(r_Z.size() == a * c);

                randomize(r_Z);
                auto mat_r_X = getMat(a, b, r_X);
                auto mat_r_Y = getMat(b, c, r_Y);
                auto mat_r_Z = getMat(a, c, r_Z);

                shark::span<u64> r_C(a * c);
                auto mat_r_C = getMat(a, c, r_C);
                // r_C = r_X @ r_Y + r_Z
                // shark::utils::matmuladd(a, b, c, r_X, r_Y, r_Z, r_C);
                mat_r_C = mat_r_X * mat_r_Y + mat_r_Z;

                send_authenticated_ashare(r_X);
                send_authenticated_ashare(r_Y);
                send_authenticated_ashare(r_C);
            }

            void eval(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                shark::utils::start_timer("key_read");
                auto [r_X, r_X_tag] = recv_authenticated_ashare(a * b);
                auto [r_Y, r_Y_tag] = recv_authenticated_ashare(b * c);
                auto [r_Z, r_Z_tag] = recv_authenticated_ashare(a * c);
                shark::utils::stop_timer("key_read");

                auto mat_X = getMat(a, b, X).cast<u128>();
                auto mat_Y = getMat(b, c, Y).cast<u128>();

                shark::span<u128> Z_share(a * c);
                shark::span<u128> Z_tag(a * c);
                auto mat_Z_share = getMat(a, c, Z_share);
                auto mat_Z_tag = getMat(a, c, Z_tag);

                auto mat_r_X = getMat(a, b, r_X);
                auto mat_r_X_tag = getMat(a, b, r_X_tag);
                auto mat_r_Y = getMat(b, c, r_Y);
                auto mat_r_Y_tag = getMat(b, c, r_Y_tag);
                auto mat_r_Z = getMat(a, c, r_Z);
                auto mat_r_Z_tag = getMat(a, c, r_Z_tag);

                // Z = r_Z + X @ Y - r_X @ Y - X @ r_Y
                mat_Z_share = mat_r_Z + (mat_X * u128(party) - mat_r_X) * mat_Y;
                mat_Z_share -= mat_X * mat_r_Y;

                mat_Z_tag = mat_r_Z_tag + (mat_X * ring_key - mat_r_X_tag) * mat_Y;
                mat_Z_tag -= mat_X * mat_r_Y_tag;

                Z = authenticated_reconstruct(Z_share, Z_tag);
            }

            void call(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &Z)
            {
                if (party == DEALER)
                {
                    gen(a, b, c, X, Y, Z);
                }
                else
                {
                    eval(a, b, c, X, Y, Z);
                }
            }

            shark::span<u64> call(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y)
            {
                shark::span<u64> Z(a * c);
                call(a, b, c, X, Y, Z);
                return Z;
            }

            shark::span<u64> emul(u64 a, u64 b, u64 c, const shark::span<u64> &X, const shark::span<u64> &Y)
            {
                shark::span<u64> Z(a * c);
                auto X_mat = getMat(a, b, X);
                auto Y_mat = getMat(b, c, Y);
                auto Z_mat = getMat(a, c, Z);
                Z_mat = X_mat * Y_mat;
                return Z;
            }
        }
    }
}