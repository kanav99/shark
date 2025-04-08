#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <cryptoTools/Crypto/RandomOracle.h>
#include <shark/utils/timer.hpp>

namespace shark
{
    namespace protocols
    {
        osuCrypto::PRNG prngGlobal;
        int party;

        Peer *server;
        Peer *client;

        Dealer *dealer;
        Peer *peer;

        u128 ring_key;
        u64 bit_key;

        std::vector<u128> batchCheckArithmBuffer;
        std::vector<u64>  batchCheckBoolBuffer;

        bool parallel_reconstruct = true;

        bool mpspdz_32bit_compaison = false;

        template block rand<block>();
        template u64 rand<u64>();
        template u128 rand<u128>();
        template std::array<block, 2> rand<std::array<block, 2>>();

        void randomize(shark::span<u128> &share)
        {
            for (u64 i = 0; i < share.size(); i++)
            {
                share[i] = rand<u64>();
            }
        }

        void randomize_high(shark::span<u128> &share)
        {
            for (u64 i = 0; i < share.size(); i++)
            {
                setHigh(share[i], rand<u64>());
            }
        }

        void randomize(shark::span<u64> &share)
        {
            for (u64 i = 0; i < share.size(); i++)
            {
                share[i] = rand<u64>();
            }
        }

        void randomize(shark::span<u8> &share)
        {
            u64 tmp = rand<u64>();
            for (u64 i = 0; i < share.size(); i++)
            {
                share[i] = (tmp >> (i % 64)) & 1;
                if (i % 64 == 63)
                {
                    tmp = rand<u64>();
                }
            }
        }

        void send_authenticated_ashare(const shark::span<u64> &share)
        {
            u64 size = share.size();
            // TODO: packing, PRG optimization
            shark::span<u128> share_0(size);
            shark::span<u128> share_1(size);
            shark::span<u128> share_0_tag(size);
            shark::span<u128> share_1_tag(size);

            randomize(share_0);
            randomize(share_0_tag);
            randomize_high(share_1);
            
            #pragma omp parallel for
            for (u64 i = 0; i < size; i++)
            {
                // shares of x
                setLow(share_1[i], share[i] - getLow(share_0[i]));
                // shares of x * ring_key
                share_1_tag[i] = ring_key * (share_0[i] + share_1[i]) - share_0_tag[i];
            }

            server->send_array(share_0);
            server->send_array(share_0_tag);
            client->send_array(share_1);
            client->send_array(share_1_tag);
        }

        std::pair<shark::span<u128>, shark::span<u128> >
        recv_authenticated_ashare(u64 size)
        {
            auto share = dealer->recv_array<u128>(size);
            auto share_tag = dealer->recv_array<u128>(size);
            auto p = std::make_pair(std::move(share), std::move(share_tag));
            return p;
        }

        void send_authenticated_bshare(const shark::span<u8> &share)
        {
            u64 size = share.size();
            // TODO: packing, PRG optimization
            shark::span<u8> share_0(size);
            shark::span<u64> share_0_tag(size);
            
            shark::span<u8> share_1(size);
            shark::span<u64> share_1_tag(size);

            randomize(share_0);
            randomize(share_0_tag);

            #pragma omp parallel for
            for (u64 i = 0; i < size; i++)
            {
                // shares of x
                share_1[i] = share[i] ^ share_0[i];
                // share of tag
                share_1_tag[i] = share_0_tag[i] ^ (share[i] * bit_key);
            }

            server->send_array(share_0);
            server->send_array(share_0_tag);
            client->send_array(share_1);
            client->send_array(share_1_tag);
        }

        shark::span<FKOS> recv_authenticated_bshare(u64 size)
        {
            shark::span<FKOS> share(size);

            auto share_bit = dealer->recv_array<u8>(size);
            auto share_tag = dealer->recv_array<u64>(size);

            for (u64 i = 0; i < size; i++)
            {
                share[i] = std::make_tuple(share_bit[i], share_tag[i]);
            }

            return share;
        }

        void send_dcfbit(const shark::span<u64> &share, int bin)
        {
            u64 size = share.size();
            for (u64 i = 0; i < size; ++i)
            {
                auto [k0, k1] = crypto::dcfbit_gen(bin, share[i]);
                server->send_array(k0.k);
                server->send_array(k0.v_bit);
                server->send_array(k0.v_tag_1);
                server->send(k0.g_bit);
                server->send(k0.g_tag_1);

                client->send_array(k1.k);
                client->send_array(k1.v_bit);
                client->send_array(k1.v_tag_1);
                client->send(k1.g_bit);
                client->send(k1.g_tag_1);
            }
        }

        void send_dcfring(const shark::span<u64> &share, int bin)
        {
            u64 size = share.size();
            for (u64 i = 0; i < size; ++i)
            {
                auto [k0, k1] = crypto::dcfring_gen(bin, share[i]);
                server->send_array(k0.k);
                server->send_array(k0.v_ring);
                server->send_array(k0.v_tag);
                server->send(k0.g_ring);
                server->send(k0.g_tag);

                client->send_array(k1.k);
                client->send_array(k1.v_ring);
                client->send_array(k1.v_tag);
                client->send(k1.g_ring);
                client->send(k1.g_tag);
            }
        }

        void send_dpfring(const shark::span<u64> &share, int bin)
        {
            u64 size = share.size();
            for (u64 i = 0; i < size; ++i)
            {
                auto [k0, k1] = crypto::dpfring_gen(bin, share[i]);
                server->send_array(k0.k);
                server->send(k0.g_ring);
                server->send(k0.g_tag);

                client->send_array(k1.k);
                client->send(k1.g_ring);
                client->send(k1.g_tag);
            }
        }

        shark::span<crypto::DCFBitKey> recv_dcfbit(u64 size, int bin)
        {
            shark::span<crypto::DCFBitKey> keys(size);
            for (u64 i = 0; i < size; ++i)
            {
                auto k = dealer->recv_array<block>(bin + 1);
                auto v_bit = dealer->recv_array<u8>(bin);
                auto v_tag_1 = dealer->recv_array<u64>(bin);
                // auto v_tag_2 = dealer->recv_array<u64>(bin);
                // auto v_tag_2 = shark::span<u64>(bin);
                u8 g_bit = dealer->recv<u8>();
                u64 g_tag_1 = dealer->recv<u64>();
                // u64 g_tag_2 = dealer->recv<u64>();

                keys[i] = std::move(
                    crypto::DCFBitKey(
                        std::move(k), 
                        std::move(v_bit), 
                        std::move(v_tag_1), 
                        // std::move(v_tag_2), 
                        g_bit, g_tag_1, 0
                    )
                );
            }
            return keys;
        }

        shark::span<crypto::DCFRingKey> recv_dcfring(u64 size, int bin)
        {
            shark::span<crypto::DCFRingKey> keys(size);
            for (u64 i = 0; i < size; ++i)
            {
                auto k = dealer->recv_array<block>(bin + 1);
                auto v_ring = dealer->recv_array<u128>(bin);
                auto v_tag = dealer->recv_array<u128>(bin);
                u128 g_ring = dealer->recv<u128>();
                u128 g_tag = dealer->recv<u128>();

                keys[i] = std::move(
                    crypto::DCFRingKey(
                        std::move(k), 
                        std::move(v_ring), 
                        std::move(v_tag), 
                        g_ring, g_tag
                    )
                );
            }
            return keys;
        }

        shark::span<crypto::DPFRingKey> recv_dpfring(u64 size, int bin)
        {
            shark::span<crypto::DPFRingKey> keys(size);
            for (u64 i = 0; i < size; ++i)
            {
                auto k = dealer->recv_array<block>(bin + 1);
                u128 g_ring = dealer->recv<u128>();
                u128 g_tag = dealer->recv<u128>();

                keys[i] = std::move(
                    crypto::DPFRingKey(
                        std::move(k), 
                        g_ring, g_tag
                    )
                );
            }
            return keys;
        }

        void authenticated_reconstruct(shark::span<u128> &share, const shark::span<u128> &share_tag, shark::span<u64> &res)
        {
            shark::utils::start_timer("reconstruct");
            shark::span<u128> tmp(share.size());

            if (parallel_reconstruct)
            {
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        peer->send_array(share);
                    }

                    #pragma omp section
                    {
                        peer->recv_array(tmp);
                    }
                }
            }
            else
            {
                peer->send_array(share);
                peer->recv_array(tmp);
            }
            for (u64 i = 0; i < share.size(); i++)
            {
                share[i] += tmp[i];
            }

            for (u64 i = 0; i < share.size(); i++)
            {
                u128 z = share[i] * ring_key - share_tag[i];
                batchCheckArithmBuffer.push_back(z);
                res[i] = getLow(share[i]);
            }
            shark::utils::stop_timer("reconstruct");
        }

        shark::span<u64> authenticated_reconstruct(shark::span<u128> &share, const shark::span<u128> &share_tag)
        {
            shark::span<u64> res(share.size());
            authenticated_reconstruct(share, share_tag, res);
            return res;
        }

        shark::span<u8> authenticated_reconstruct(shark::span<FKOS> &share)
        {
            shark::utils::start_timer("reconstruct");
            shark::span<u8> tmp_bit(share.size());
            // shark::span<u64> tmp_M(share.size());
            shark::span<u8> share_bit(share.size());
            // shark::span<u64> share_K(share.size());
            // shark::span<u64> share_M(share.size());

            // TODO: unnecesary copy
            for (u64 i = 0; i < share.size(); i++)
            {
                share_bit[i] = std::get<0>(share[i]);
                // share_K[i] = std::get<1>(share[i]);
                // share_M[i] = std::get<2>(share[i]);
            }

            if (parallel_reconstruct)
            {
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        peer->send_array(share_bit);
                        // peer->send_array(share_M);
                    }

                    #pragma omp section
                    {
                        peer->recv_array(tmp_bit);
                        // peer->recv_array(tmp_M);
                    }
                }
            }
            else
            {
                peer->send_array(share_bit);
                // peer->send_array(share_M);
                peer->recv_array(tmp_bit);
                // peer->recv_array(tmp_M);
            }

            // TODO: batch check like ring shares
            for (u64 i = 0; i < share.size(); i++)
            {
                // always_assert(tmp_M[i] == share_K[i] ^ (tmp_bit[i] * bit_key[party]));
                share_bit[i] ^= tmp_bit[i];
                u64 z = std::get<1>(share[i]) ^ (share_bit[i] * bit_key);
                batchCheckBoolBuffer.push_back(z);
            }
            shark::utils::stop_timer("reconstruct");
            return share_bit;
        }

        template <typename T>
        shark::span<u8> compute_commitment(int party, T x, block r)
        {
            osuCrypto::RandomOracle H;
            shark::span<u8> commitment(osuCrypto::RandomOracle::HashSize);
            H.Update((u8 *)&party, sizeof(int));
            H.Update((u8 *)&x, sizeof(T));
            H.Update((u8 *)&r, sizeof(block));
            H.Final(commitment.data());
            return commitment;
        }

        template <typename T>
        T commit_and_exchange(T &x)
        {
            block r = rand<block>();
            shark::span<u8> commitment = compute_commitment(party, x, r);
            peer->send_array(commitment);
            auto peer_commitment = peer->recv_array<u8>(osuCrypto::RandomOracle::HashSize);

            peer->send(x);
            T peer_x = peer->recv<T>();

            shark::span<u8> peer_commitment2 = compute_commitment(SERVER + CLIENT - party, peer_x, r);

            for (int i = 0; i < osuCrypto::RandomOracle::HashSize; i++)
            {
                always_assert(peer_commitment[i] == peer_commitment2[i]);
            }

            return peer_x;
        }

        void batch_check()
        {

            if ((batchCheckArithmBuffer.size() == 0) && (batchCheckBoolBuffer.size() == 0))
            {
                return;
            }

            osuCrypto::PRNG prngBatchCheck;
            block r_prng = rand<block>();
            block peer_r_prng = commit_and_exchange(r_prng);
            block prng_seed = r_prng ^ peer_r_prng;
            prngBatchCheck.SetSeed(prng_seed);

            u128 batchCheckAccumulated = 0;
            u128 batchCheckBitsAccumulated = 0;
            for (u64 i = 0; i < batchCheckArithmBuffer.size(); i++)
            {
                batchCheckAccumulated += (u128(prngBatchCheck.get<u64>()) * batchCheckArithmBuffer[i]);
            }

            for (u64 i = 0; i < batchCheckBoolBuffer.size(); i++)
            {
                batchCheckBitsAccumulated += prngBatchCheck.get<u64>() * batchCheckBoolBuffer[i];
            }

            // commit and open batchCheckAccumulated
            auto batchCheckPair = std::make_pair(batchCheckAccumulated, batchCheckBitsAccumulated);
            auto peer_batchCheckPair = commit_and_exchange(batchCheckPair);

            always_assert(batchCheckAccumulated + peer_batchCheckPair.first == 0);
            always_assert(batchCheckBitsAccumulated == peer_batchCheckPair.second);
            batchCheckArithmBuffer.clear();
            batchCheckBoolBuffer.clear();
        }

        // FKOS arithmetic
        FKOS xor_fkos(FKOS x, FKOS y)
        {
            return std::make_tuple(std::get<0>(x) ^ std::get<0>(y), std::get<1>(x) ^ std::get<1>(y));
        }

        FKOS not_fkos(FKOS x)
        {
            if (party == SERVER)
            {
                return std::make_tuple(std::get<0>(x) ^ 1, std::get<1>(x) ^ bit_key);
            }
            else
            {
                return std::make_tuple(std::get<0>(x), std::get<1>(x) ^ bit_key);
            }
        }
    }
}