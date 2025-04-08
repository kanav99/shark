#include <shark/protocols/maxpool.hpp>
#include <shark/protocols/relu.hpp>
#include <shark/protocols/common.hpp>

namespace shark
{
    namespace protocols
    {
        namespace maxpool
        {
            void _max(const shark::span<u64> &X, const shark::span<u64> &Y, shark::span<u64> &res)
            {
                always_assert(X.size() == Y.size());
                always_assert(X.size() == res.size());
                
                shark::span<u64> tmp(X.size());

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); ++i)
                {
                    tmp[i] = X[i] - Y[i];
                }

                auto reluout = relu::call(tmp);

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); ++i)
                {
                    res[i] = reluout[i] + Y[i];
                }
            }

            void _logmax(u64 s1, u64 s2, const shark::span<u64> &X, shark::span<u64> &Z)
            {
                always_assert(X.size() == s1 * s2);
                always_assert(Z.size() == s1);

                shark::span<u64> res(s1 * s2);

                #pragma omp parallel for collapse(2)
                for (u64 i = 0; i < s1; ++i)
                {
                    for (u64 j = 0; j < s2; ++j)
                    {
                        res[i * s2 + j] = X[i * s2 + j];
                    }
                }

                u64 curr = s2;
                while (curr != 1)
                {
                    u64 curr2 = curr / 2;

                    shark::span<u64> left(s1 * curr2);
                    shark::span<u64> right(s1 * curr2);

                    #pragma omp parallel for collapse(2)
                    for (u64 i = 0; i < s1; ++i)
                    {
                        for (u64 j = 0; j < curr2; ++j)
                        {
                            // Arr2DIdx(left, s1, curr2, i, j) = Arr2DIdx(res, s1, curr, i, 2 * j);
                            left[i * curr2 + j] = res[i * curr + 2 * j];
                            // Arr2DIdx(right, s1, curr2, i, j) = Arr2DIdx(res, s1, curr, i, 2 * j + 1);
                            right[i * curr2 + j] = res[i * curr + 2 * j + 1];
                        }
                    }

                    _max(left, right, left);

                    u64 currNext;
                    if (curr % 2 == 0)
                    {
                        currNext = curr / 2;
                    }
                    else
                    {
                        currNext = curr / 2 + 1;
                    }

                    shark::span<u64> resNext(s1 * currNext);

                    if (curr % 2 == 1)
                    {
                        #pragma omp parallel for
                        for (u64 i = 0; i < s1; ++i)
                        {
                            resNext[i * currNext + currNext - 1] = res[i * curr + curr - 1];
                        }
                    }

                    #pragma omp parallel for collapse(2)
                    for (u64 i = 0; i < s1; ++i)
                    {
                        for (u64 j = 0; j < curr2; ++j)
                        {
                            resNext[i * currNext + j] = left[i * curr2 + j];
                        }
                    }

                    res = resNext;
                    curr = currNext;
                }

                #pragma omp parallel for
                for (u64 i = 0; i < s1; ++i)
                {
                    Z[i] = res[i];
                }
            }

            shark::span<u64> _reshapeMaxpool(u64 f, u64 padding, u64 stride, u64 ci, u64 inH, u64 inW, const shark::span<u64> &Img)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                u64 outH = (inH + 2 * padding - f) / stride + 1;
                u64 outW = (inW + 2 * padding - f) / stride + 1;
                u64 s1 = bs * outH * outW * ci;
                u64 s2 = f * f;

                shark::span<u64> X(s1 * s2);

                using i64 = int64_t;

                #pragma omp parallel for collapse(6)
                for (u64 _bs = 0; _bs < bs; ++_bs)
                {
                    for (u64 _h = 0; _h < outH; ++_h)
                    {
                        for (u64 _w = 0; _w < outW; ++_w)
                        {
                            for (u64 _ci = 0; _ci < ci; ++_ci)
                            {
                                for (u64 _fh = 0; _fh < f; ++_fh)
                                {
                                    for (u64 _fw = 0; _fw < f; ++_fw)
                                    {
                                        u64 i = _bs * outH * outW * ci + _h * outW * ci + _w * ci + _ci;
                                        u64 j = _fh * f + _fw;
                                        i64 inX = _h * stride + _fh - padding;
                                        i64 inY = _w * stride + _fw - padding;
                                        
                                        if (inX < 0 || inX >= inH || inY < 0 || inY >= inW)
                                        {
                                            X[i * s2 + j] = 0;
                                        }
                                        else
                                        {
                                            u64 k = _bs * inH * inW * ci + (inX) * inW * ci + (inY) * ci + _ci;
                                            X[i * s2 + j] = Img[k];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                return X;
            }

            void call(u64 f, u64 padding, u64 stride, u64 ci, u64 inH, u64 inW, const shark::span<u64> &Img, shark::span<u64> &OutImg)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                u64 outH = (inH + 2 * padding - f) / stride + 1;
                u64 outW = (inW + 2 * padding - f) / stride + 1;
                always_assert(OutImg.size() == bs * outH * outW * ci);

                u64 s1 = bs * outH * outW * ci;
                u64 s2 = f * f;

                auto X = _reshapeMaxpool(f, padding, stride, ci, inH, inW, Img);
                _logmax(s1, s2, X, OutImg);

            }

            shark::span<u64> call(u64 f, u64 padding, u64 stride, u64 ci, u64 inH, u64 inW, const shark::span<u64> &Img)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                u64 outH = (inH + 2 * padding - f) / stride + 1;
                u64 outW = (inW + 2 * padding - f) / stride + 1;

                shark::span<u64> OutImg(bs * outH * outW * ci);

                call(f, padding, stride, ci, inH, inW, Img, OutImg);

                return OutImg;
            }
        }
    }
}