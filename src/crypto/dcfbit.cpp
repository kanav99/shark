#include <shark/utils/assert.hpp>
#include <shark/types/u64.hpp>
#include <shark/protocols/common.hpp>
#include <shark/crypto/dcfbit.hpp>
#include <cryptoTools/Crypto/AES.h>

namespace shark
{
    namespace crypto
    {
        using namespace osuCrypto;

        const block notOneBlock = toBlock(~0, ~1);
        const block notThreeBlock = toBlock(~0, ~3);
        const block ZeroBlock = toBlock(0, 0);
        const block OneBlock = toBlock(0, 1);
        const block TwoBlock = toBlock(0, 2);
        const block ThreeBlock = toBlock(0, 3);
        const block pt[4] = {ZeroBlock, OneBlock, TwoBlock, ThreeBlock};
        const int tag_size = 40;

        const AES ak0(ZeroBlock);
        const AES ak1(OneBlock);
        const AES ak2(TwoBlock);
        const AES ak3(ThreeBlock);

        inline u8 lsb(const block &b)
        {
            return _mm_cvtsi128_si64x(b) & 1;
        }

        void convert(const block &in, u8 &out_bit, u64 &out_tag_1, u64 &out_tag_2)
        {
            u64 * in_ptr = (u64 *) &in;
            out_bit = lsb(in);
            out_tag_1 = (in_ptr[0] >> 1) & ((1ull << tag_size) - 1);
            out_tag_2 = (in_ptr[1] >> 1) & ((1ull << tag_size) - 1);
        }

        std::pair<DCFBitKey, DCFBitKey> dcfbit_gen(int bin, const u64 alpha, const bool greaterThan)
        {
            u8 payload_bit = 1;
            u64 payload_tag_1 = shark::protocols::bit_key;
            u64 payload_tag_2 = shark::protocols::bit_key;

            auto s = protocols::rand<std::array<block, 2>>();
            block si[2][2];
            block vi[2][2];

            u8 v_alpha_bit = 0;
            u64 v_alpha_tag_1 = 0;
            u64 v_alpha_tag_2 = 0;

            shark::span<block> k0(bin + 1);
            shark::span<block> k1(bin + 1);
            
            shark::span<u8> v0_bit(bin);
            shark::span<u64> v0_tag_1(bin);
            shark::span<u64> v0_tag_2(bin);

            s[0] = (s[0] & notOneBlock) ^ ((s[1] & OneBlock) ^ OneBlock);
            k0[0] = s[0];
            k1[0] = s[1];
            
            block ct[4];
            u8 vi_00_converted_bit;
            u64 vi_00_converted_tag_1;
            u64 vi_00_converted_tag_2;
            u8 vi_01_converted_bit;
            u64 vi_01_converted_tag_1;
            u64 vi_01_converted_tag_2;
            u8 vi_10_converted_bit;
            u64 vi_10_converted_tag_1;
            u64 vi_10_converted_tag_2;
            u8 vi_11_converted_bit;
            u64 vi_11_converted_tag_1;
            u64 vi_11_converted_tag_2;

            for (int i = 0; i < bin; ++i)
            {
                const u8 keep = static_cast<uint8_t>(alpha >> (bin - 1 - i)) & 1;
                auto a = toBlock(keep);

                auto ss0 = s[0] & notThreeBlock;
                auto ss1 = s[1] & notThreeBlock;

                // AES ak0(ss0);
                // AES ak1(ss1);
                // ak0.ecbEncFourBlocks(pt, ct);
                si[0][0] = ak0.ecbEncBlock(ss0) ^ ss0;
                si[0][1] = ak1.ecbEncBlock(ss0) ^ ss0;
                vi[0][0] = ak2.ecbEncBlock(ss0) ^ ss0;
                vi[0][1] = ak3.ecbEncBlock(ss0) ^ ss0;
                // ak1.ecbEncFourBlocks(pt, ct);
                si[1][0] = ak0.ecbEncBlock(ss1) ^ ss1;
                si[1][1] = ak1.ecbEncBlock(ss1) ^ ss1;
                vi[1][0] = ak2.ecbEncBlock(ss1) ^ ss1;
                vi[1][1] = ak3.ecbEncBlock(ss1) ^ ss1;

                auto ti0 = lsb(s[0]);
                auto ti1 = lsb(s[1]);

                convert(vi[0][keep], vi_00_converted_bit, vi_00_converted_tag_1, vi_00_converted_tag_2);
                convert(vi[1][keep], vi_10_converted_bit, vi_10_converted_tag_1, vi_10_converted_tag_2);
                convert(vi[0][keep ^ 1], vi_01_converted_bit, vi_01_converted_tag_1, vi_01_converted_tag_2);
                convert(vi[1][keep ^ 1], vi_11_converted_bit, vi_11_converted_tag_1, vi_11_converted_tag_2);

                // for (int lp = 0; lp < groupSize; ++lp)
                // {
                v0_bit[i] = v_alpha_bit ^ vi_01_converted_bit ^ vi_11_converted_bit;
                v0_tag_1[i] = v_alpha_tag_1 ^ vi_01_converted_tag_1 ^ vi_11_converted_tag_1;
                v0_tag_2[i] = v_alpha_tag_2 ^ vi_01_converted_tag_2 ^ vi_11_converted_tag_2;
                if (keep == 0 && greaterThan)
                {
                    // Lose is R
                    v0_bit[i] = v0_bit[i] ^ payload_bit;
                    v0_tag_1[i] ^= payload_tag_1;
                    v0_tag_2[i] ^= payload_tag_2;
                }
                else if (keep == 1 && !greaterThan)
                {
                    // Lose is L
                    v0_bit[i] = v0_bit[i] ^ payload_bit;
                    v0_tag_1[i] ^= payload_tag_1;
                    v0_tag_2[i] ^= payload_tag_2;
                }
                v_alpha_bit = v_alpha_bit ^ vi_10_converted_bit ^ vi_00_converted_bit ^ v0_bit[i];
                v_alpha_tag_1 = v_alpha_tag_1 ^ vi_10_converted_tag_1 ^ vi_00_converted_tag_1 ^ v0_tag_1[i];
                v_alpha_tag_2 = v_alpha_tag_2 ^ vi_10_converted_tag_2 ^ vi_00_converted_tag_2 ^ v0_tag_2[i];
                // }

                std::array<block, 2> siXOR{si[0][0] ^ si[1][0], si[0][1] ^ si[1][1]};

                // get the left and right t_CW bits
                std::array<block, 2> t{
                    (OneBlock & siXOR[0]) ^ a ^ OneBlock,
                    (OneBlock & siXOR[1]) ^ a};

                // take scw to be the bits [127, 2] as scw = s0_loss ^ s1_loss
                auto scw = siXOR[keep ^ 1] & notThreeBlock;

                k0[i + 1] = k1[i + 1] = scw           // set bits [127, 2] as scw = s0_loss ^ s1_loss
                                        ^ (t[0] << 1) // set bit 1 as tL
                                        ^ t[1];       // set bit 0 as tR

                auto si0Keep = si[0][keep];
                auto si1Keep = si[1][keep];

                // extract the t^Keep_CW bit
                auto TKeep = t[keep];

                // set the next level of s,t
                s[0] = si0Keep ^ (zeroAndAllOne[ti0] & (scw ^ TKeep));
                s[1] = si1Keep ^ (zeroAndAllOne[ti1] & (scw ^ TKeep));
            }

            u8 s0_converted_bit;
            u64 s0_converted_tag_1;
            u64 s0_converted_tag_2;
            u8 s1_converted_bit;
            u64 s1_converted_tag_1;
            u64 s1_converted_tag_2;
            // convert(Bout, groupSize, s[0] & notThreeBlock, s0_converted);
            convert(s[0] & notThreeBlock, s0_converted_bit, s0_converted_tag_1, s0_converted_tag_2);
            // convert(Bout, groupSize, s[1] & notThreeBlock, s1_converted);
            convert(s[1] & notThreeBlock, s1_converted_bit, s1_converted_tag_1, s1_converted_tag_2);

            u8 g0_bit = s1_converted_bit ^ s0_converted_bit ^ v_alpha_bit;
            u64 g0_tag_1 = s1_converted_tag_1 ^ s0_converted_tag_1 ^ v_alpha_tag_1;
            u64 g0_tag_2 = s1_converted_tag_2 ^ s0_converted_tag_2 ^ v_alpha_tag_2;

            return std::make_pair(
                DCFBitKey(k0, v0_bit, v0_tag_1, g0_bit, g0_tag_1, g0_tag_2),
                DCFBitKey(k1, v0_bit, v0_tag_1, g0_bit, g0_tag_1, g0_tag_2)
            );
        }

        block traverseOneDCF(const block &s, const block &cw, const u8 &keep,
                        u8 &out_bit, u64 &out_tag_1, u64 &out_tag_2,
                        u8 v_bit, u64 v_tag_1, //u64 v_tag_2,
                        bool geq)

        {
            static const block blocks[4] = {ZeroBlock, TwoBlock, OneBlock, ThreeBlock};

            block stcw;
            block ct[2]; // {tau, v_this_level}
            u8 t_previous = lsb(s);
            const auto scw = (cw & notThreeBlock);
            block ds[] = { ((cw >> 1) & OneBlock), (cw & OneBlock) };
            const auto mask = zeroAndAllOne[t_previous];
            auto ss = s & notThreeBlock;

            // AES ak(ss);
            if (keep == 0)
            {
                ct[0] = ak0.ecbEncBlock(ss) ^ ss;
                ct[1] = ak2.ecbEncBlock(ss) ^ ss;
            }
            else
            {
                ct[0] = ak1.ecbEncBlock(ss) ^ ss;
                ct[1] = ak3.ecbEncBlock(ss) ^ ss;
            }
            // ak.ecbEncTwoBlocks(blocks + 2 * keep, ct);

            stcw = ((scw ^ ds[keep]) & mask) ^ ct[0];

            u8 v_this_level_converted_bit;
            u64 v_this_level_converted_tag_1;
            u64 v_this_level_converted_tag_2;
            convert(ct[1], v_this_level_converted_bit, v_this_level_converted_tag_1, v_this_level_converted_tag_2);
            out_bit = out_bit ^ (v_this_level_converted_bit ^ t_previous * v_bit);
            out_tag_1 = out_tag_1 ^ (v_this_level_converted_tag_1 ^ t_previous * v_tag_1);
            // out_tag_2 = out_tag_2 ^ (v_this_level_converted_tag_2 ^ t_previous * v_tag_2);
            return stcw;
        }

        std::tuple<block, u8, u64, u64> traversePathDCF(const DCFBitKey &key, u64 x, const bool geq)
        {
            int bin = key.k.size() - 1;
            block s = _mm_loadu_si128(key.k.data());
            u8 out_bit = 0;
            u64 out_tag_1 = 0;
            u64 out_tag_2 = 0;

            for (int i = 0; i < bin; ++i)
            {
                const u8 keep = static_cast<uint8_t>(x >> (bin - 1 - i)) & 1;
                s = traverseOneDCF(s, _mm_loadu_si128(key.k.data() + (i + 1)), 
                        keep, 
                        out_bit, out_tag_1, out_tag_2, 
                        key.v_bit[i], key.v_tag_1[i],
                        geq);
            }
            return std::make_tuple(s, out_bit, out_tag_1, out_tag_2);
        }


        std::tuple<u8, u64> dcfbit_eval(const DCFBitKey &key, const u64 &x, const bool greaterThan)
        {
            auto [s, out_bit, out_tag_1, out_tag_2] = traversePathDCF(key, x, greaterThan);
            u8 t = lsb(s);

            u8 s_converted_bit;
            u64 s_converted_tag_1;
            u64 s_converted_tag_2;

            convert(s & notThreeBlock, s_converted_bit, s_converted_tag_1, s_converted_tag_2);

            if (t)
            {
                s_converted_bit = s_converted_bit ^ key.g_bit;
                s_converted_tag_1 = s_converted_tag_1 ^ key.g_tag_1;
                s_converted_tag_2 = s_converted_tag_2 ^ key.g_tag_2;
            }

            out_bit = out_bit ^ s_converted_bit;
            out_tag_1 = out_tag_1 ^ s_converted_tag_1;
            out_tag_2 = out_tag_2 ^ s_converted_tag_2;

            return std::make_tuple(out_bit, out_tag_1);
        }

    }
}
