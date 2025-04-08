#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>

#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/maxpool.hpp>
#include <shark/protocols/add.hpp>
#include <shark/protocols/ars.hpp>

#include <shark/utils/timer.hpp>
#include <shark/utils/assert.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

void fill(shark::span<u64> &X, u64 val)
{
    for (u64 i = 0; i < X.size(); i++)
        X[i] = val;
}

class Loader
{
    std::ifstream file;
public:
    Loader(const std::string &filename)
    {
        file.open(filename, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file " << filename << std::endl;
            std::exit(1);
        }
    }

    void load(shark::span<u64> &X, int f)
    {
        int size = X.size();
        for (int i = 0; i < size; i++)
        {
            float fval;
            file.read((char *)&fval, sizeof(float));
            X[i] = (u64)(fval * (1ull << f));
        }   
    }

    ~Loader()
    {
        file.close();
    }
};

void emul();

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    if (party == EMUL)
    {
        emul();
        return 0;
    }

    u64 batch_size = 1;
    u64 image_size = 32 * 32 * 3;
    int f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(3 * 64 * 5 * 5);
    shark::span<u64> B1(64);
    shark::span<u64> F2(64 * 64 * 5 * 5);
    shark::span<u64> B2(64);
    shark::span<u64> F3(64 * 64 * 5 * 5);
    shark::span<u64> B3(64);
    shark::span<u64> W4(64 * 10);
    shark::span<u64> B4(10);

    if (party == CLIENT)
    {
        Loader loader("ml/datasets/cifar10/images/0.bin");
        loader.load(Image, f);
    }
    
    if (party == SERVER)
    {
        Loader loader("ml/weights/hinet/weights.dat");
        loader.load(F1, f);
        loader.load(B1, 2*f);
        loader.load(F2, f);
        loader.load(B2, 2*f);
        loader.load(F3, f);
        loader.load(B3, 2*f);
        loader.load(W4, f);
        loader.load(B4, 2*f);
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(F1, SERVER);
    input::call(B1, SERVER);
    input::call(F2, SERVER);
    input::call(B2, SERVER);
    input::call(F3, SERVER);
    input::call(B3, SERVER);
    input::call(W4, SERVER);
    input::call(B4, SERVER);
    shark::utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();
    shark::utils::start_timer("hinet");

    // shark::utils::start_timer("100");
    auto a1 = conv::call(5, 1, 1, 3, 64, 32, 32, Image, F1);
    auto a2 = add::call(a1, B1);
    // shark::utils::stop_timer("100");
    // shark::utils::start_timer("101");
    auto a3 = maxpool::call(3, 0, 2, 64, 30, 30, a2);
    // shark::utils::stop_timer("101");
    // shark::utils::start_timer("102");
    auto a4 = relutruncate::call(a3, f);
    // shark::utils::stop_timer("102");

    // shark::utils::start_timer("103");
    auto a5 = conv::call(5, 1, 1, 64, 64, 14, 14, a4, F2);
    auto a6 = add::call(a5, B2);
    // shark::utils::stop_timer("103");
    // shark::utils::start_timer("104");
    auto a7 = maxpool::call(3, 0, 2, 64, 12, 12, a6);
    // shark::utils::stop_timer("104");
    // shark::utils::start_timer("105");
    auto a8 = relutruncate::call(a7, f);
    // shark::utils::stop_timer("105");

    // shark::utils::start_timer("106");
    auto a9 = conv::call(5, 1, 1, 64, 64, 5, 5, a8, F3);
    auto a10 = add::call(a9, B3);
    // shark::utils::stop_timer("106");
    // shark::utils::start_timer("107");
    auto a11 = maxpool::call(3, 0, 2, 64, 3, 3, a10);
    // shark::utils::stop_timer("107");
    // shark::utils::start_timer("108");
    auto a12 = relutruncate::call(a11, f);
    // shark::utils::stop_timer("108");

    // shark::utils::start_timer("109");
    auto a13 = matmul::call(batch_size, 64, 10, a12, W4);
    auto a14 = add::call(a13, B4);
    // shark::utils::stop_timer("109");
    // shark::utils::start_timer("110");
    auto a15 = ars::call(a14, f);
    // shark::utils::stop_timer("110");

    output::call(a15);
    if (party != DEALER)
    {
        for (int i = 0; i < 10; i++)
        {
            std::cout << (float(int64_t(a15[i])) / (1ull << f)) << " ";
        }
    }

    shark::utils::stop_timer("hinet");

    finalize::call();

    shark::utils::print_all_timers();
}

void emul()
{
    u64 batch_size = 1;
    u64 image_size = 32 * 32 * 3;
    int f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(3 * 64 * 5 * 5);
    shark::span<u64> B1(64);
    shark::span<u64> F2(64 * 64 * 5 * 5);
    shark::span<u64> B2(64);
    shark::span<u64> F3(64 * 64 * 5 * 5);
    shark::span<u64> B3(64);
    shark::span<u64> W4(64 * 10);
    shark::span<u64> B4(10);

    {
        Loader loader("ml/datasets/cifar10/images/0.bin");
        loader.load(Image, f);
    }
    
    {
        Loader loader("ml/weights/hinet/weights.dat");
        loader.load(F1, f);
        loader.load(B1, 2*f);
        loader.load(F2, f);
        loader.load(B2, 2*f);
        loader.load(F3, f);
        loader.load(B3, 2*f);
        loader.load(W4, f);
        loader.load(B4, 2*f);
    }

    auto a1 = conv::emul(5, 1, 1, 3, 64, 32, 32, Image, F1);
    auto a2 = add::call(a1, B1);
    auto _a2 = ars::emul(a2, f);
    std::cout << (float(int64_t(_a2[0])) / (1ull << f)) << std::endl;
    std::cout << (float(int64_t(B1[0])) / (1ull << (2*f))) << std::endl;
}
