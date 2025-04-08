#include <shark/protocols/input.hpp>
#include <shark/protocols/common.hpp>

namespace shark
{
    namespace protocols
    {
        namespace output
        {
            void gen(const shark::span<u64> &r_X)
            {
                send_authenticated_ashare(r_X);
            }

            void gen(const shark::span<u8> &r_X)
            {
                send_authenticated_bshare(r_X);
            }

            void eval(shark::span<u64> &X)
            {
                auto [r_X, r_X_tag] = recv_authenticated_ashare(X.size());
                batch_check();
                auto r = authenticated_reconstruct(r_X, r_X_tag);
                batch_check();

                for (u64 i = 0; i < X.size(); i++)
                {
                    X[i] -= r[i];
                }
            }

            void eval(shark::span<u8> &X)
            {
                auto r_bdoz = recv_authenticated_bshare(X.size());
                batch_check();
                auto r = authenticated_reconstruct(r_bdoz);
                batch_check();

                for (u64 i = 0; i < X.size(); i++)
                {
                    X[i] ^= r[i];
                }
            }

            void call(shark::span<u64> &X)
            {
                if (party == DEALER)
                {
                    gen(X);
                }
                else
                {
                    eval(X);
                }
            }

            void call(shark::span<u8> &X)
            {
                if (party == DEALER)
                {
                    gen(X);
                }
                else
                {
                    eval(X);
                }
            }
        }
    }
}
