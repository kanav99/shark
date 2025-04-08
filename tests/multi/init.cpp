#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/common.hpp>

using namespace shark::protocols;

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    finalize::call();
}
