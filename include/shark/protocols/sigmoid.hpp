#pragma once

#include <shark/protocols/spline.hpp>
#include <shark/protocols/lrs.hpp>
#include <vector>

namespace shark
{
    namespace protocols
    {
        namespace sigmoid
        {
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
                32767 + 1, // fix: x = 32767 will give very wrong answer, increase all knots by 1?
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
                auto Y_intermediate = spline::call(f + 4, 2, knots2, polynomials2, X);
                
                int sin = f, scoef = 20, sout = f, degree = 2;
                lrs::call(Y_intermediate, Y, degree * sin + scoef - sout);
            }

            inline shark::span<u64> call(int f, const shark::span<u64> &X)
            {
                always_assert(f >= 12);
                shark::span<u64> Y(X.size());
                call(f, X, Y);
                return Y;
            }
        }
    }
}