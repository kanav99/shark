#pragma once

#include <memory>
#include <fstream>

namespace shark {
    namespace protocols {
        namespace init {
            void gen(uint64_t key);
            void eval(int party, std::string ip, int port, bool oneShot = true);
            void from_args(int argc, char ** argv);
        }
    }
}