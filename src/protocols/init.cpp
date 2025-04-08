#include <shark/protocols/common.hpp>
#include <shark/protocols/init.hpp>
#include <shark/utils/assert.hpp>
#include <fstream>

namespace shark {
    namespace protocols {

        namespace init {
            void gen(uint64_t key)
            {
                party = DEALER;
                prngGlobal.SetSeed(osuCrypto::toBlock(key));

                server = new Peer(filename[0]);
                client = new Peer(filename[1]);

                u64 ring_key_0 = rand<u64>();
                u64 ring_key_1 = rand<u64>();
                ring_key = ring_key_0;
                ring_key += ring_key_1;
                server->send(ring_key_0);
                client->send(ring_key_1);

                u64 bit_key_0 = rand<u64>();
                u64 bit_key_1 = rand<u64>();
                bit_key = bit_key_0 ^ bit_key_1;

                server->send(bit_key_0);
                client->send(bit_key_1);

            }

            void eval(int _party, std::string ip, int port, bool oneShot)
            {
                always_assert(_party == SERVER || _party == CLIENT);
                party = _party;
                dealer = new Dealer(filename[party], oneShot);

                ring_key = dealer->recv<u64>();
                bit_key = dealer->recv<u64>();

                // setup communication between evaluating parties
                if (party == SERVER)
                {
                    peer = waitForPeer(port);
                }
                else
                {
                    peer = new Peer(ip, port);
                }

                prngGlobal.SetSeed(osuCrypto::toBlock(::rand(), ::rand()));

            }

            void from_args(int argc, char ** argv)
            {
                always_assert(argc > 1);
                int _party = atoi(argv[1]);
                always_assert(_party == DEALER || _party == SERVER || _party == CLIENT || _party == EMUL);

                if (_party == EMUL)
                {
                    party = EMUL;
                    return;
                }
                
                std::string ip = (argc > 2) ? argv[2] : "127.0.0.1";
                int port = (argc > 3) ? atoi(argv[3]) : 42069;
                if (_party == DEALER)
                    init::gen(0xdeadbeef);
                else
                    eval(_party, ip, 42069, true);
            }
        }
    }
}
