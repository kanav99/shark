#include <shark/utils/assert.hpp>
#include <shark/types/u128.hpp>
#include <shark/protocols/common.hpp>
#include <shark/crypto/dcfring.hpp>
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

        const AES ak0(toBlock(11898517819052905481ull, 11033408630406285283ull));
        const AES ak1(toBlock(9326388341714895150ull, 10128647480701028797ull));
        const AES ak2(toBlock(9873267564496987689ull, 9447730648146089691ull));
        const AES ak3(toBlock(10184333836464677069ull, 9405075354919185247ull));
        const AES ak4(toBlock(9778003634410817943ull, 12187153955522203843ull));
        const AES ak5(toBlock(13266479233780938046ull, 13416694524136657701ull));

        inline u8 lsb(const block &b)
        {
            return _mm_cvtsi128_si64x(b) & 1;
        }

        void convert(const block &in, u128 &out_ring, u128 &out_tag)
        {
            block ct[2];
            // AES ak(in);
            // ak.ecbEncTwoBlocks(pt, ct);
            ct[0] = ak4.ecbEncBlock(in) ^ in;
            ct[1] = ak5.ecbEncBlock(in) ^ in;
            out_ring = *(u128 *)ct;
            out_tag = *(u128 *)(ct + 1);
        }

        std::pair<DCFRingKey, DCFRingKey> dcfring_gen(int bin, const u64 alpha, const bool greaterThan)
        {
            u128 payload_ring = 1; // Optimization: Need not randomize top s bits, DCF output always added with fresh shares
            u128 payload_tag = shark::protocols::ring_key;

            auto s = protocols::rand<std::array<block, 2>>();
            block si[2][2];
            block vi[2][2];

            u128 v_alpha_ring = 0;
            u128 v_alpha_tag = 0;

            shark::span<block> k0(bin + 1);
            shark::span<block> k1(bin + 1);
            
            shark::span<u128> v0_ring(bin);
            shark::span<u128> v0_tag(bin);

            s[0] = (s[0] & notOneBlock) ^ ((s[1] & OneBlock) ^ OneBlock);
            k0[0] = s[0];
            k1[0] = s[1];
            
            block ct[4];
            u128 vi_00_converted_ring;
            u128 vi_00_converted_tag;
            u128 vi_01_converted_ring;
            u128 vi_01_converted_tag;
            u128 vi_10_converted_ring;
            u128 vi_10_converted_tag;
            u128 vi_11_converted_ring;
            u128 vi_11_converted_tag;

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
                u128 sign = 1;
                if (ti1 == 1) sign -= 2;

                convert(vi[0][keep], vi_00_converted_ring, vi_00_converted_tag);
                convert(vi[1][keep], vi_10_converted_ring, vi_10_converted_tag);
                convert(vi[0][keep ^ 1], vi_01_converted_ring, vi_01_converted_tag);
                convert(vi[1][keep ^ 1], vi_11_converted_ring, vi_11_converted_tag);

                // this block needs to be edited
                v0_ring[i] = sign * (-v_alpha_ring - vi_01_converted_ring + vi_11_converted_ring);
                v0_tag[i] = sign * (-v_alpha_tag - vi_01_converted_tag + vi_11_converted_tag);
                if ((keep == 0) && greaterThan)
                {
                    // Lose is R
                    v0_ring[i] += sign * payload_ring;
                    v0_tag[i] += sign * payload_tag;
                }
                else if ((keep == 1) && !greaterThan)
                {
                    // Lose is L
                    v0_ring[i] += sign * payload_ring;
                    v0_tag[i] += sign * payload_tag;
                }
                v_alpha_ring = v_alpha_ring - vi_10_converted_ring + vi_00_converted_ring + sign * v0_ring[i];
                v_alpha_tag = v_alpha_tag - vi_10_converted_tag + vi_00_converted_tag + sign * v0_tag[i];

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

            u128 s0_converted_ring;
            u128 s0_converted_tag;
            u128 s1_converted_ring;
            u128 s1_converted_tag;
            // convert(Bout, groupSize, s[0] & notThreeBlock, s0_converted);
            convert(s[0] & notThreeBlock, s0_converted_ring, s0_converted_tag);
            // convert(Bout, groupSize, s[1] & notThreeBlock, s1_converted);
            convert(s[1] & notThreeBlock, s1_converted_ring, s1_converted_tag);

            // this block needs to be edited
            u128 g0_ring = s1_converted_ring - s0_converted_ring - v_alpha_ring;
            u128 g0_tag = s1_converted_tag - s0_converted_tag - v_alpha_tag;
            if (lsb(s[1]) == 1)
            {
                g0_ring = -g0_ring;
                g0_tag = -g0_tag;
            }

            return std::make_pair(
                DCFRingKey(k0, v0_ring, v0_tag, g0_ring, g0_tag),
                DCFRingKey(k1, v0_ring, v0_tag, g0_ring, g0_tag)
            );
        }

        block traverseOneDCF(const block &s, const block &cw, const u8 &keep,
                        u128 &out_ring, u128 &out_tag,
                        u128 v_ring, u128 v_tag,
                        bool geq, int party)

        {
            static const block blocks[4] = {ZeroBlock, TwoBlock, OneBlock, ThreeBlock};

            block stcw;
            block ct[2]; // {tau, v_this_level}
            u8 t_previous = lsb(s);
            // std::cout << "t[i] = " << int(t_previous);
            // std::cout << " s[i] = " << s;
            // std::cout << " v[i] = " << v_ring << std::endl;
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

            u128 v_this_level_converted_ring;
            u128 v_this_level_converted_tag;
            convert(ct[1], v_this_level_converted_ring, v_this_level_converted_tag);
            // this block needs to be edited 
            u128 sign = 1;
            if (party == 1) sign -= 2;
            // std::cout << "sign: " << sign << std::endl;
            out_ring = out_ring + sign * (v_this_level_converted_ring + v_ring * t_previous);
            // std::cout << "out_ring: " << out_ring << std::endl;
            out_tag = out_tag + sign * (v_this_level_converted_tag + v_tag * t_previous);
            return stcw;
        }

        std::tuple<block, u128, u128> traversePathDCF(int party, const DCFRingKey &key, u64 x, const bool geq)
        {
            int bin = key.k.size() - 1;
            block s = _mm_loadu_si128(key.k.data());
            u128 out_ring = 0;
            u128 out_tag = 0;

            for (int i = 0; i < bin; ++i)
            {
                const u8 keep = static_cast<uint8_t>(x >> (bin - 1 - i)) & 1;
                s = traverseOneDCF(s, _mm_loadu_si128(key.k.data() + (i + 1)), 
                        keep, 
                        out_ring, out_tag, 
                        key.v_ring[i], key.v_tag[i],
                        geq, party);
            }
            return std::make_tuple(s, out_ring, out_tag);
        }


        std::tuple<u128, u128> dcfring_eval(int party, const DCFRingKey &key, const u64 &x, const bool greaterThan)
        {
            auto [s, out_ring, out_tag] = traversePathDCF(party, key, x, greaterThan);
            // std::cout << "s: " << s << std::endl;
            u8 t = lsb(s);

            u128 s_converted_ring;
            u128 s_converted_tag;

            convert(s & notThreeBlock, s_converted_ring, s_converted_tag);

            // this block needs to be edited
            if (t)
            {
                s_converted_ring = s_converted_ring + key.g_ring;
                s_converted_tag = s_converted_tag + key.g_tag;
            }

            if (party == 1)
            {
                s_converted_ring = -s_converted_ring;
                s_converted_tag = -s_converted_tag;
            }

            out_ring = out_ring + s_converted_ring;
            out_tag = out_tag + s_converted_tag;

            return std::make_tuple(out_ring, out_tag);
        }
    }
}
