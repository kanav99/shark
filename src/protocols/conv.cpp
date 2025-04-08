#include <shark/types/u128.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>
#include <shark/utils/eigen.hpp>

using namespace shark::matrix;

namespace shark {
    namespace protocols {
        namespace conv {

            template <typename T>
            shark::span<T> getReshapedImage(u64 F, u64 padding, u64 stride, u64 CI, u64 inH, u64 inW, const shark::span<T> &Img)
            {
                // shape of Img is [bs, inH, inW, ci]
                always_assert(Img.size() % (inH * inW * CI) == 0);
                u64 BS = Img.size() / (inH * inW * CI);
                u64 outH = (inH - F + 2 * padding) / stride + 1;
                u64 outW = (inW - F + 2 * padding) / stride + 1;
                u64 reshapedImgCols = BS * outH * outW;
                u64 reshapedImgRows = F * F * CI;
                shark::span<T> reshapedImg(reshapedImgRows * reshapedImgCols);

                using i64 = int64_t;
                u64 linIdxFilterMult = 0;
                for (i64 n = 0; n < BS; n++){
		            i64 leftTopCornerH = 0 - padding;
		            i64 extremeRightBottomCornerH = inH - 1 + padding;
		            while((leftTopCornerH + F - 1) <= extremeRightBottomCornerH)
                    { 
			            i64 leftTopCornerW = 0 - padding;
			            i64 extremeRightBottomCornerW = inW - 1 + padding;
			            while((leftTopCornerW + F - 1) <= extremeRightBottomCornerW)
                        {

				            for (i64 fh = 0; fh < F; fh++)
                            {
					            for (i64 fw = 0; fw < F; fw++)
                                {
						            i64 curPosH = leftTopCornerH + fh;
						            i64 curPosW = leftTopCornerW + fw;
						            for (i64 _ci = 0; _ci < CI; _ci++)
                                    {
                                        u64 rowIdx = (fh*F*CI) + (fw*CI) + _ci;
							            if ((((curPosH < 0) || (curPosH >= inH)) || ((curPosW < 0) || (curPosW >= inW))))
                                        {
								            // reshapedImg(linIdxFilterMult, rowIdx) = 0L;
                                            reshapedImg[rowIdx * reshapedImgCols + linIdxFilterMult] = 0L;
							            }
							            else
                                        {
								            // reshapedImg(linIdxFilterMult, rowIdx) = input(n, curPosH, curPosW, _ci);
                                            reshapedImg[rowIdx * reshapedImgCols + linIdxFilterMult] = Img[n * inH * inW * CI + curPosH * inW * CI + curPosW * CI + _ci];

							            }
						            }
					            }
				            }

				            linIdxFilterMult = linIdxFilterMult + 1;
				            leftTopCornerW = leftTopCornerW + stride;
			            }

            			leftTopCornerH = leftTopCornerH + stride;
		            }
            	}

                return reshapedImg;
                // return getMat(reshapedImgRows, reshapedImgCols, reshapedImg);
            }

            template <typename T>
            shark::span<T> getReshapedOutput(u64 BS, u64 H, u64 W, u64 C, const shark::span<T> &Z)
            {
                // shape of Z is [bs, h, w, c]
                // reshape to [c, bs * h * w]
                always_assert(Z.size() == BS * H * W * C);
                shark::span<T> reshapedZ(C * BS * H * W);
                for (u64 i = 0; i < BS; i++)
                {
                    for (u64 j = 0; j < H; j++)
                    {
                        for (u64 k = 0; k < W; k++)
                        {
                            for (u64 l = 0; l < C; l++)
                            {
                                reshapedZ[l * BS * H * W + i * H * W + j * W + k] = Z[i * H * W * C + j * W * C + k * C + l];
                            }
                        }
                    }
                }
                return reshapedZ;
            }

            template <typename T>
            void getReshapedOutputReversed(u64 BS, u64 H, u64 W, u64 C, const shark::span<T> &reshapedZ, shark::span<T> &Z)
            {
                // shape of Z is [bs, h, w, c]
                // reshape to [c, bs * h * w]
                always_assert(reshapedZ.size() == BS * H * W * C);
                always_assert(Z.size() == BS * H * W * C);

                for (u64 i = 0; i < BS; i++)
                {
                    for (u64 j = 0; j < H; j++)
                    {
                        for (u64 k = 0; k < W; k++)
                        {
                            for (u64 l = 0; l < C; l++)
                            {
                                Z[i * H * W * C + j * W * C + k * C + l] = reshapedZ[l * BS * H * W + i * H * W + j * W + k];
                            }
                        }
                    }
                }
            }

            void gen(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &r_Img, const shark::span<u64> &r_Filter, shark::span<u64> &r_Z)
            {
                always_assert(r_Img.size() % (inH * inW * ci) == 0);
                u64 bs = r_Img.size() / (inH * inW * ci);
                always_assert(r_Filter.size() == co * f * f * ci);
                u64 outH = (inH - f + 2 * padding) / stride + 1;
                u64 outW = (inW - f + 2 * padding) / stride + 1;
                always_assert(r_Z.size() == bs * outH * outW * co);

                randomize(r_Z);
                auto mat_r_Filter = getMat(co, f * f * ci, r_Filter);
                auto reshaped_r_Img = getReshapedImage(f, padding, stride, ci, inH, inW, r_Img);
                auto mat_r_Img = getMat(f * f * ci, bs * outH * outW, reshaped_r_Img);
                auto reshaped_r_Z = getReshapedOutput(bs, outH, outW, co, r_Z);
                auto mat_r_Z = getMat(co, bs * outH * outW, reshaped_r_Z);

                shark::span<u64> r_C(co * bs * outH * outW);
                auto mat_r_C = getMat(co, bs * outH * outW, r_C);
                // r_C = r_X @ r_Y + r_Z
                // shark::utils::matmuladd(a, b, c, r_X, r_Y, r_Z, r_C);
                mat_r_C = mat_r_Filter * mat_r_Img + mat_r_Z;

                send_authenticated_ashare(r_Img);
                send_authenticated_ashare(r_Filter);
                send_authenticated_ashare(r_C);
            }

            void eval(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter, shark::span<u64> &Z)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                always_assert(Filter.size() == co * f * f * ci);
                u64 outH = (inH - f + 2 * padding) / stride + 1;
                u64 outW = (inW - f + 2 * padding) / stride + 1;
                always_assert(Z.size() == bs * outH * outW * co);

                shark::utils::start_timer("key_read");
                auto [r_Img, r_Img_tag] = recv_authenticated_ashare(bs * inH * inW * ci);
                auto [r_Filter, r_Filter_tag] = recv_authenticated_ashare(co * f * f * ci);
                auto [r_C, r_C_tag] = recv_authenticated_ashare(co * bs * outH * outW);
                shark::utils::stop_timer("key_read");

                // reshapes and casts
                auto reshaped_Img = getReshapedImage(f, padding, stride, ci, inH, inW, Img);
                auto reshaped_r_Img = getReshapedImage(f, padding, stride, ci, inH, inW, r_Img);
                auto reshaped_r_Img_tag = getReshapedImage(f, padding, stride, ci, inH, inW, r_Img_tag);

                auto mat_Img = getMat(f * f * ci, bs * outH * outW, reshaped_Img).cast<u128>();
                auto mat_r_Img = getMat(f * f * ci, bs * outH * outW, reshaped_r_Img);
                auto mat_r_Img_tag = getMat(f * f * ci, bs * outH * outW, reshaped_r_Img_tag);

                shark::span<u128> reshaped_Z(co * bs * outH * outW);
                shark::span<u128> reshaped_Z_tag(co * bs * outH * outW);
                auto mat_reshaped_Z = getMat(co, bs * outH * outW, reshaped_Z);
                auto mat_reshaped_Z_tag = getMat(co, bs * outH * outW, reshaped_Z_tag);

                auto mat_Filter = getMat(co, f * f * ci, Filter).cast<u128>();
                auto mat_r_Filter = getMat(co, f * f * ci, r_Filter);
                auto mat_r_Filter_tag = getMat(co, f * f * ci, r_Filter_tag);
                
                auto mat_r_C = getMat(co, bs * outH * outW, r_C);
                auto mat_r_C_tag = getMat(co, bs * outH * outW, r_C_tag);

                // real computation
                // Z = r_Z + X @ Y - r_X @ Y - X @ r_Y
                mat_reshaped_Z = mat_r_C + (mat_Filter * party - mat_r_Filter) * mat_Img;
                mat_reshaped_Z -= mat_Filter * mat_r_Img;

                mat_reshaped_Z_tag = mat_r_C_tag + (mat_Filter * ring_key - mat_r_Filter_tag) * mat_Img;
                mat_reshaped_Z_tag -= mat_Filter * mat_r_Img_tag;

                auto Z_reconstructed = authenticated_reconstruct(reshaped_Z, reshaped_Z_tag);
                getReshapedOutputReversed(bs, outH, outW, co, Z_reconstructed, Z);
            }

            void call(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter, shark::span<u64> &Z)
            {
                if (party == DEALER)
                {
                    gen(f, padding, stride, ci, co, inH, inW, Img, Filter, Z);
                }
                else
                {
                    eval(f, padding, stride, ci, co, inH, inW, Img, Filter, Z);
                }
            }
 
            shark::span<u64> call(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                always_assert(Filter.size() == f * f * ci * co);
                u64 outH = (inH - f + 2 * padding) / stride + 1;
                u64 outW = (inW - f + 2 * padding) / stride + 1;

                shark::span<u64> Z(bs * outH * outW * co);
                call(f, padding, stride, ci, co, inH, inW, Img, Filter, Z);
                return Z;
            }

            shark::span<u64> emul(u64 f, u64 padding, u64 stride, u64 ci, u64 co, u64 inH, u64 inW, const shark::span<u64> &Img, const shark::span<u64> &Filter)
            {
                always_assert(Img.size() % (inH * inW * ci) == 0);
                u64 bs = Img.size() / (inH * inW * ci);
                always_assert(Filter.size() == f * f * ci * co);
                u64 outH = (inH - f + 2 * padding) / stride + 1;
                u64 outW = (inW - f + 2 * padding) / stride + 1;

                shark::span<u64> Z(bs * outH * outW * co);
                shark::span<u64> reshaped_Z(co * bs * outH * outW);
                auto reshaped_Img = getReshapedImage(f, padding, stride, ci, inH, inW, Img);
                auto mat_Img = getMat(f * f * ci, bs * outH * outW, reshaped_Img);
                auto mat_Filter = getMat(co, f * f * ci, Filter);
                auto mat_reshaped_Z = getMat(co, bs * outH * outW, reshaped_Z);
                mat_reshaped_Z = mat_Filter * mat_Img;
                getReshapedOutputReversed(bs, outH, outW, co, reshaped_Z, Z);
                return Z;
            }
        }
    }
}