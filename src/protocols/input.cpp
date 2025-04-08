#include <shark/protocols/input.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        namespace input
        {
            void gen(shark::span<u64> &r_X, int owner)
            {
                randomize(r_X);
                
                if (owner == SERVER)
                {
                    server->send_array(r_X);
                }
                else
                {
                    client->send_array(r_X);
                }
            }

            void eval(shark::span<u64> &X, int owner)
            {
                if (owner == party)
                {
                    shark::utils::start_timer("key_read-input");
                    auto r_X_clear = dealer->recv_array<u64>(X.size());
                    shark::utils::stop_timer("key_read-input");

                    for (u64 i = 0; i < X.size(); i++)
                    {
                        X[i] += r_X_clear[i];
                    }

                    peer->send_array(X);
                }
                else
                {
                    peer->recv_array(X);
                }
            }

            void call(shark::span<u64> &X, int owner)
            {
                if (party == DEALER)
                {
                    gen(X, owner);
                }
                else
                {
                    eval(X, owner);
                }
            }

            void gen(shark::span<u8> &r_X, int owner)
            {
                randomize(r_X);
                
                if (owner == SERVER)
                {
                    server->send_array(r_X);
                }
                else
                {
                    client->send_array(r_X);
                }
            }

            void eval(shark::span<u8> &X, int owner)
            {
                if (owner == party)
                {
                    shark::utils::start_timer("key_read-input");
                    auto r_X_clear = dealer->recv_array<u8>(X.size());
                    shark::utils::stop_timer("key_read-input");

                    for (u64 i = 0; i < X.size(); i++)
                    {
                        X[i] ^= r_X_clear[i];
                    }

                    peer->send_array(X);
                }
                else
                {
                    peer->recv_array(X);
                }
            }

            void call(shark::span<u8> &X, int owner)
            {
                if (party == DEALER)
                {
                    gen(X, owner);
                }
                else
                {
                    eval(X, owner);
                }
            }
        }
    }
}