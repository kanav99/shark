#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>

#include <shark/protocols/relutruncate.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/maxpool.hpp>
#include <shark/protocols/add.hpp>

#include <shark/utils/timer.hpp>
#include <shark/utils/assert.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

void fill(shark::span<u64> &X, u64 val)
{
    for (u64 i = 0; i < X.size(); i++)
        X[i] = val;
}

void conv_update(u64 &h, u64 &w)
{
    u64 f = 3;
    u64 padding = 1;
    u64 stride = 1;
    h = (h - f + 2 * padding) / stride + 1;
    w = (w - f + 2 * padding) / stride + 1;
}

void maxpool_update(u64 &h, u64 &w)
{
    u64 f = 2;
    u64 padding = 0;
    u64 stride = 2;
    h = (h - f + 2 * padding) / stride + 1;
    w = (w - f + 2 * padding) / stride + 1;
}

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 batch_size = 1;
    u64 h = 224;
    u64 w = 224;
    u64 c = 3;
    u64 image_size = h * w * c;
    u64 f = 16;

    shark::span<u64> Image(batch_size * image_size);
    
    shark::span<u64> F1(3 * 64 * 3 * 3);
    shark::span<u64> F2(64 * 64 * 3 * 3);
    shark::span<u64> F3(64 * 128 * 3 * 3);
    shark::span<u64> F4(128 * 128 * 3 * 3);
    shark::span<u64> F5(128 * 256 * 3 * 3);
    shark::span<u64> F6(256 * 256 * 3 * 3);
    shark::span<u64> F7(256 * 256 * 3 * 3);
    shark::span<u64> F8(256 * 512 * 3 * 3);
    shark::span<u64> F9(512 * 512 * 3 * 3);
    shark::span<u64> F10(512 * 512 * 3 * 3);
    shark::span<u64> F11(512 * 512 * 3 * 3);
    shark::span<u64> F12(512 * 512 * 3 * 3);
    shark::span<u64> F13(512 * 512 * 3 * 3);
    shark::span<u64> W14(25088 * 4096);
    shark::span<u64> W15(4096 * 4096);
    shark::span<u64> W16(4096 * 1000);

    shark::span<u64> B1(64);
    shark::span<u64> B2(64);
    shark::span<u64> B3(64);
    shark::span<u64> B4(64);
    shark::span<u64> B5(64);
    shark::span<u64> B6(64);
    shark::span<u64> B7(64);
    shark::span<u64> B8(64);
    shark::span<u64> B9(64);
    shark::span<u64> B10(64);
    shark::span<u64> B11(64);
    shark::span<u64> B12(64);
    shark::span<u64> B13(64);
    shark::span<u64> B14(4096);
    shark::span<u64> B15(4096);
    shark::span<u64> B16(1000);

    if (party == CLIENT)
        fill(Image, 1 << f);
    
    if (party == SERVER)
    {
        fill(F1, 1 << f);
        fill(F2, 1 << f);
        fill(F3, 1 << f);
        fill(F4, 1 << f);
        fill(F5, 1 << f);
        fill(F6, 1 << f);
        fill(F7, 1 << f);
        fill(F8, 1 << f);
        fill(F9, 1 << f);
        fill(F10, 1 << f);
        fill(F11, 1 << f);
        fill(F12, 1 << f);
        fill(F13, 1 << f);
        fill(W14, 1 << f);
        fill(W15, 1 << f);
        fill(W16, 1 << f);
        fill(B1, 1 << (2*f));
        fill(B2, 1 << (2*f));
        fill(B3, 1 << (2*f));
        fill(B4, 1 << (2*f));
        fill(B5, 1 << (2*f));
        fill(B6, 1 << (2*f));
        fill(B7, 1 << (2*f));
        fill(B8, 1 << (2*f));
        fill(B9, 1 << (2*f));
        fill(B10, 1 << (2*f));
        fill(B11, 1 << (2*f));
        fill(B12, 1 << (2*f));
        fill(B13, 1 << (2*f));
        fill(B14, 1 << (2*f));
        fill(B15, 1 << (2*f));
        fill(B16, 1 << (2*f));
    }

    shark::utils::start_timer("input");
    input::call(Image, CLIENT);
    input::call(F1, SERVER);
    input::call(F2, SERVER);
    input::call(F3, SERVER);
    input::call(F4, SERVER);
    input::call(F5, SERVER);
    input::call(F6, SERVER);
    input::call(F7, SERVER);
    input::call(F8, SERVER);
    input::call(F9, SERVER);
    input::call(F10, SERVER);
    input::call(F11, SERVER);
    input::call(F12, SERVER);
    input::call(F13, SERVER);
    input::call(W14, SERVER);
    input::call(W15, SERVER);
    input::call(W16, SERVER);
    input::call(B1, SERVER);
    input::call(B2, SERVER);
    input::call(B3, SERVER);
    input::call(B4, SERVER);
    input::call(B5, SERVER);
    input::call(B6, SERVER);
    input::call(B7, SERVER);
    input::call(B8, SERVER);
    input::call(B9, SERVER);
    input::call(B10, SERVER);
    input::call(B11, SERVER);
    input::call(B12, SERVER);
    input::call(B13, SERVER);
    input::call(B14, SERVER);
    input::call(B15, SERVER);
    input::call(B16, SERVER);
    shark::utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();

    shark::utils::start_timer("vgg16");

    // rt
    // shark::utils::start_timer("100");
    auto a1 = conv::call(3, 1, 1, 3, 64, h, w, Image, F1);
    conv_update(h, w);
    auto a2 = add::call(a1, B1);
    // shark::utils::stop_timer("100");
    // shark::utils::start_timer("101");
    auto a3 = relutruncate::call(a2, f);
    // shark::utils::stop_timer("101");

    // mp+rt
    // shark::utils::start_timer("102");
    auto a4 = conv::call(3, 1, 1, 64, 64, h, w, a3, F2);
    conv_update(h, w);
    auto a5 = add::call(a4, B2);
    // shark::utils::stop_timer("102");
    // shark::utils::start_timer("103");
    auto a6 = maxpool::call(2, 0, 2, 64, h, w, a5);
    maxpool_update(h, w);
    // shark::utils::stop_timer("103");
    // shark::utils::start_timer("104");
    auto a7 = relutruncate::call(a6, f);
    // shark::utils::stop_timer("104");

    // rt
    // shark::utils::start_timer("105");
    auto a8 = conv::call(3, 1, 1, 64, 128, h, w, a7, F3);
    conv_update(h, w);
    auto a9 = add::call(a8, B3);
    // shark::utils::stop_timer("105");
    // shark::utils::start_timer("106");
    auto a10 = relutruncate::call(a9, f);
    // shark::utils::stop_timer("106");

    // mp+rt
    // shark::utils::start_timer("107");
    auto a11 = conv::call(3, 1, 1, 128, 128, h, w, a10, F4);
    conv_update(h, w);
    auto a12 = add::call(a11, B4);
    // shark::utils::stop_timer("107");
    // shark::utils::start_timer("108");
    auto a13 = maxpool::call(2, 0, 2, 128, h, w, a12);
    maxpool_update(h, w);
    // shark::utils::stop_timer("108");
    // shark::utils::start_timer("109");
    auto a14 = relutruncate::call(a13, f);
    // shark::utils::stop_timer("109");

    // rt
    // shark::utils::start_timer("110");
    auto a15 = conv::call(3, 1, 1, 128, 256, h, w, a14, F5);
    conv_update(h, w);
    auto a16 = add::call(a15, B5);
    // shark::utils::stop_timer("110");
    // shark::utils::start_timer("111");
    auto a17 = relutruncate::call(a16, f);
    // shark::utils::stop_timer("111");

    // rt
    // shark::utils::start_timer("112");
    auto a18 = conv::call(3, 1, 1, 256, 256, h, w, a17, F6);
    conv_update(h, w);
    auto a19 = add::call(a18, B6);
    // shark::utils::stop_timer("112");
    // shark::utils::start_timer("113");
    auto a20 = relutruncate::call(a19, f);
    // shark::utils::stop_timer("113");

    // mp+rt
    // shark::utils::start_timer("114");
    auto a21 = conv::call(3, 1, 1, 256, 256, h, w, a20, F7);
    conv_update(h, w);
    auto a22 = add::call(a21, B7);
    // shark::utils::stop_timer("114");
    // shark::utils::start_timer("115");
    auto a23 = maxpool::call(2, 0, 2, 256, h, w, a22);
    maxpool_update(h, w);
    // shark::utils::stop_timer("115");
    // shark::utils::start_timer("116");
    auto a24 = relutruncate::call(a23, f);
    // shark::utils::stop_timer("116");

    // rt
    // shark::utils::start_timer("117");
    auto a25 = conv::call(3, 1, 1, 256, 512, h, w, a24, F8);
    conv_update(h, w);
    auto a26 = add::call(a25, B8);
    // shark::utils::stop_timer("117");
    // shark::utils::start_timer("118");
    auto a27 = relutruncate::call(a26, f);
    // shark::utils::stop_timer("118");

    // rt
    // shark::utils::start_timer("119");
    auto a28 = conv::call(3, 1, 1, 512, 512, h, w, a27, F9);
    conv_update(h, w);
    auto a29 = add::call(a28, B9);
    // shark::utils::stop_timer("119");
    // shark::utils::start_timer("120");
    auto a30 = relutruncate::call(a29, f);
    // shark::utils::stop_timer("120");

    // mp+rt
    // shark::utils::start_timer("121");
    auto a31 = conv::call(3, 1, 1, 512, 512, h, w, a30, F10);
    conv_update(h, w);
    auto a32 = add::call(a31, B10);
    // shark::utils::stop_timer("121");
    // shark::utils::start_timer("122");
    auto a33 = maxpool::call(2, 0, 2, 512, h, w, a32);
    maxpool_update(h, w);
    // shark::utils::stop_timer("122");
    // shark::utils::start_timer("123");
    auto a34 = relutruncate::call(a33, f);
    // shark::utils::stop_timer("123");

    // rt
    // shark::utils::start_timer("124");
    auto a35 = conv::call(3, 1, 1, 512, 512, h, w, a34, F11);
    conv_update(h, w);
    auto a36 = add::call(a35, B11);
    // shark::utils::stop_timer("124");
    // shark::utils::start_timer("125");
    auto a37 = relutruncate::call(a36, f);
    // shark::utils::stop_timer("125");

    // rt
    // shark::utils::start_timer("126");
    auto a38 = conv::call(3, 1, 1, 512, 512, h, w, a37, F12);
    conv_update(h, w);
    auto a39 = add::call(a38, B12);
    // shark::utils::stop_timer("126");
    // shark::utils::start_timer("127");
    auto a40 = relutruncate::call(a39, f);
    // shark::utils::stop_timer("127");

    // mp+rt
    // shark::utils::start_timer("128");
    auto a41 = conv::call(3, 1, 1, 512, 512, h, w, a40, F13);
    conv_update(h, w);
    auto a42 = add::call(a41, B13);
    // shark::utils::stop_timer("128");
    // shark::utils::start_timer("129");
    auto a43 = maxpool::call(2, 0, 2, 512, h, w, a42);
    maxpool_update(h, w);
    // shark::utils::stop_timer("129");
    // shark::utils::start_timer("130");
    auto a44 = relutruncate::call(a43, f);
    // shark::utils::stop_timer("130");

    // fc
    // shark::utils::start_timer("131");
    auto a45 = matmul::call(batch_size, 25088, 4096, a44, W14);
    auto a46 = add::call(a45, B14);
    // shark::utils::stop_timer("131");
    // shark::utils::start_timer("132");
    auto a47 = relutruncate::call(a46, f);
    // shark::utils::stop_timer("132");

    // fc
    // shark::utils::start_timer("133");
    auto a48 = matmul::call(batch_size, 4096, 4096, a47, W15);
    auto a49 = add::call(a48, B15);
    // shark::utils::stop_timer("133");
    // shark::utils::start_timer("134");
    auto a50 = relutruncate::call(a49, f);
    // shark::utils::stop_timer("134");

    // fc
    // shark::utils::start_timer("135");
    auto a51 = matmul::call(batch_size, 4096, 1000, a50, W16);
    auto a52 = add::call(a51, B16);
    // shark::utils::stop_timer("135");
    // shark::utils::start_timer("136");
    auto a53 = relutruncate::call(a52, f);
    // shark::utils::stop_timer("136");

    output::call(a53);
    shark::utils::stop_timer("vgg16");
    // if (party != DEALER)
    //     std::cout << a53 << std::endl;


    finalize::call();

    shark::utils::print_all_timers();

}
