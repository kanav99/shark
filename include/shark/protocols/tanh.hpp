#pragma once

#include <shark/protocols/spline.hpp>
#include <shark/protocols/ars.hpp>
#include <vector>

namespace shark
{
    namespace protocols
    {
        namespace tanh
        {
            u64 i2big(int64_t x)
            {
                return x;
            }

            const std::vector<u64> knots = {
                1537,
                3075,
                4613,
                6151,
                7689,
                9227,
                10765,
                12303,
                13841,
                15379,
                16917,
                18455,
                32767,
                0ull - 18455,
                0ull - 16918,
                0ull - 15380,
                0ull - 13842,
                0ull - 12304,
                0ull - 10766,
                0ull - 9228,
                0ull - 7690,
                0ull - 6152,
                0ull - 4614,
                0ull - 3076,
                0ull - 1538
            };

            const std::vector<u64> polynomials = {
                i2big(               0),   i2big(    1100283904) , i2big(  -49070) ,
                i2big(   -125023813632),   i2big(    1262866432) , i2big( -101928) ,
                i2big(     -6375342080),   i2big(    1185718272) , i2big(  -89387) ,
                i2big(    690868977664),   i2big(     883470336) , i2big(  -56632) ,
                i2big(   1670306070528),   i2big(     565043200) , i2big(  -30750) ,
                i2big(   2571108352000),   i2big(     330747904) , i2big(  -15516) ,
                i2big(   3248807215104),   i2big(     183861248) , i2big(   -7557) ,
                i2big(   3704962940928),   i2big(      99115008) , i2big(   -3621) ,
                i2big(   3992574754816),   i2big(      52363264) , i2big(   -1721) ,
                i2big(   4165950504960),   i2big(      27312128) , i2big(    -816) ,
                i2big(   4267939201024),   i2big(      14049280) , i2big(    -384) ,
                i2big(   4324612636672),   i2big(       7348224) , i2big(    -186) ,
                i2big(   4398046511104),   i2big(             0) , i2big(       0) ,
                i2big(  -4398046511104),   i2big(             0) , i2big(       0) ,
                i2big(  -4324629413888),   i2big(       7348224) , i2big(     185) ,
                i2big(  -4267955978240),   i2big(      14049280) , i2big(     383) ,
                i2big(  -4165967282176),   i2big(      27312128) , i2big(     815) ,
                i2big(  -3992591532032),   i2big(      52363264) , i2big(    1720) ,
                i2big(  -3704979718144),   i2big(      99115008) , i2big(    3620) ,
                i2big(  -3248823992320),   i2big(     183861248) , i2big(    7556) ,
                i2big(  -2571125129216),   i2big(     330747904) , i2big(   15515) ,
                i2big(  -1670322847744),   i2big(     565043200) , i2big(   30749) ,
                i2big(   -690885754880),   i2big(     883470336) , i2big(   56631) ,
                i2big(      6358564864),   i2big(    1185718272) , i2big(   89386) ,
                i2big(    125007036416),   i2big(    1262866432) , i2big(  101927) ,
                i2big(               0),   i2big(    1100283904) , i2big(   49069)
            };

            inline void call(int f, const shark::span<u64> &X, shark::span<u64> &Y)
            {
                always_assert(f >= 12);
                std::vector<u64> knots2(knots.size());
                for (int i = 0; i < knots.size(); i++)
                {
                    knots2[i] = knots[i] * (1ull << (f - 12));
                    knots2[i] = knots2[i] % (1ull << (f + 4));
                }
                
                std::vector<u64> polynomials2(polynomials.size());
                for (int i = 0; i < polynomials.size()/3; i++)
                {
                    polynomials2[3*i] = polynomials[3*i] * (1ull << (2*f - 24));
                    polynomials2[3*i+1] = polynomials[3*i+1] * (1ull << (f - 12));
                    polynomials2[3*i+2] = polynomials[3*i+2];
                }
                auto Y_intermediate = spline::call(f+4, 2, knots2, polynomials2, X);
                
                int sin = f, scoef = 18, sout = f, degree = 2;
                ars::call(Y_intermediate, Y, degree * sin + scoef - sout);
            }

            inline shark::span<u64> call(int f, const shark::span<u64> &X)
            {
                always_assert(f>= 12);
                shark::span<u64> Y(X.size());
                call(f, X, Y);
                return Y;
            }
        }
    }
}