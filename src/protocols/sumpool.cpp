#include <shark/protocols/maxpool.hpp>
#include <shark/protocols/relu.hpp>
#include <shark/protocols/common.hpp>

namespace shark
{
    namespace protocols
    {
        namespace sumpool
        {
            using i64 = int64_t;

            void _sumpool(u64 f, u64 padding, u64 stride, u64 ci, u64 inH, u64 inW, const shark::span<u64> &Img, shark::span<u64> &OutImg)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                u64 outH = (inH + 2 * padding - f) / stride + 1;
                u64 outW = (inW + 2 * padding - f) / stride + 1;
                always_assert(OutImg.size() == bs * outH * outW * ci);

                #pragma omp parallel for collapse(4)
                for (u64 _bs = 0; _bs < bs; ++_bs)
                {
                    for (u64 _h = 0; _h < outH; ++_h)
                    {
                        for (u64 _w = 0; _w < outW; ++_w)
                        {
                            for (u64 _ci = 0; _ci < ci; ++_ci)
                            {
                                u64 i = _bs * outH * outW * ci + _h * outW * ci + _w * ci + _ci;
                                OutImg[i] = 0;
                                
                                for (u64 _fh = 0; _fh < f; ++_fh)
                                {
                                    for (u64 _fw = 0; _fw < f; ++_fw)
                                    {
                                        u64 j = _fh * f + _fw;
                                        i64 inX = _h * stride + _fh - padding;
                                        i64 inY = _w * stride + _fw - padding;
                                        
                                        if (!(inX < 0 || inX >= inH || inY < 0 || inY >= inW))
                                        {
                                            u64 k = _bs * inH * inW * ci + (inX) * inW * ci + (inY) * ci + _ci;
                                            OutImg[i] += Img[k];
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }

            void call(u64 f, u64 padding, u64 stride, u64 ci, u64 inH, u64 inW, const shark::span<u64> &Img, shark::span<u64> &OutImg)
            {
                _sumpool(f, padding, stride, ci, inH, inW, Img, OutImg);

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