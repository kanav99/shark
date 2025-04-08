#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/sigmoid.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>

using u64 = shark::u64;
using namespace shark::protocols;

u64 i2big(int64_t x)
{
    return x;
}

const std::vector<u64> knots = {
    3640 + 1,
    7281 + 1,
    10922 + 1,
    14563 + 1,
    18204 + 1,
    21845 + 1,
    25486 + 1,
    29127 + 1,
    32767 + 1,
    0ull - 32768 + 1,
    0ull - 29128 + 1,
    0ull - 25487 + 1,
    0ull - 21846 + 1,
    0ull - 18205 + 1,
    0ull - 14564 + 1,
    0ull - 10923 + 1,
    0ull - 7282 + 1,
    0ull - 3641 + 1
};

const std::vector<u64> polynomials = {
    i2big(    8796093022208ll) ,   i2big(    1111248896ll) , i2big(   -28859ll) ,
    i2big(    8485815189504ll) ,   i2big(    1281687552ll) , i2big(   -52266ll) ,
    i2big(    9286558154752ll) ,   i2big(    1061756928ll) , i2big(   -37164ll) ,
    i2big(   11421727326208ll) ,   i2big(     670793728ll) , i2big(   -19267ll) ,
    i2big(   13666132951040ll) ,   i2big(     362573824ll) , i2big(    -8685ll) ,
    i2big(   15310753103872ll) ,   i2big(     181891072ll) , i2big(    -3723ll) ,
    i2big(   16346595196928ll) ,   i2big(      87056384ll) , i2big(    -1552ll) ,
    i2big(   16937522298880ll) ,   i2big(      40681472ll) , i2big(     -643ll) ,
    i2big(   17252564860928ll) ,   i2big(      19050496ll) , i2big(     -271ll) ,
    i2big(                0ll) ,   i2big(             0ll) , i2big(        0ll) ,
    i2big(     339604406272ll) ,    i2big(     19050496ll) , i2big(      270ll) ,
    i2big(     654646968320ll) ,    i2big(     40681472ll) , i2big(      642ll) ,
    i2big(    1245574070272ll) ,    i2big(     87056384ll) , i2big(     1551ll) ,
    i2big(    2281416163328ll) ,    i2big(    181891072ll) , i2big(     3722ll) ,
    i2big(    3926036316160ll) ,    i2big(    362573824ll) , i2big(     8684ll) ,
    i2big(    6170441940992ll) ,    i2big(    670793728ll) , i2big(    19266ll) ,
    i2big(    8305611112448ll) ,    i2big(   1061756928ll) , i2big(    37163ll) ,
    i2big(    9106354077696ll) ,    i2big(   1281687552ll) , i2big(    52265ll) ,
    i2big(    8796093022208ll) ,    i2big(   1111248896ll) , i2big(    28858ll) 
};

int main(int argc, char **argv)
{
    init::from_args(argc, argv);

    u64 n = 100000;
    shark::span<u64> X(n);

    for (u64 i = 0; i < n; i++)
        X[i] = 0;

    if (party != DEALER)
        peer->sync();

    shark::utils::start_timer("spline");
    auto Y = spline::call(64, 2, knots, polynomials, X);
    auto Z = lrs::call(Y, 36);
    shark::utils::stop_timer("spline");
    output::call(Z);
    
    finalize::call();
    shark::utils::print_all_timers();
}
