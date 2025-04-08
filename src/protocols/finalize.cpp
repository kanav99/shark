#include <shark/protocols/finalize.hpp>

namespace shark
{
    namespace protocols
    {
        namespace finalize
        {
            void gen()
            {
                server->close();
                client->close();
            }

            void eval()
            {
                dealer->close();
                peer->close();
            }

            void call()
            {
                if (party == DEALER)
                {
                    gen();
                }
                else
                {
                    eval();
                }
            }
        }
        
    }
    
}
