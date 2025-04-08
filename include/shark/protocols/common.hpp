#pragma once

#include <cryptoTools/Crypto/PRNG.h>

#include <shark/types/u128.hpp>
#include <shark/utils/comm.hpp>
#include <shark/crypto/dcfbit.hpp>
#include <shark/crypto/dcfring.hpp>
#include <shark/crypto/dpfring.hpp>

namespace shark {
    namespace protocols {
        /// this key is used to generate the keys for the two parties
        /// same key to this PRNG generates exactly same keys
        /// evaluators also use this object to generate commitment randomness
        extern osuCrypto::PRNG prngGlobal;
        /// party identifier
        extern int party;
        /// legend to party identifier
        enum Party {
            SERVER = 0,
            CLIENT = 1,
            DEALER = 2,
            EMUL = 3,
        };
        /// filenames of the keyfile for the two parties
        const std::string filename[2] = {"server.dat", "client.dat"};
        /// communication structs used by the dealer
        extern Peer *server;
        extern Peer *client;
        /// communication structs used by evaluators
        extern Dealer *dealer;
        extern Peer *peer;

        /// MAC key for SPDZ_sk style shares. For dealer, this is clear value. For evaluators, this is secret share of the clear value.
        extern u128 ring_key;
        /// MAC key for FKOS style shares. For dealer, this is clear value. For evaluators, this is secret share of the clear value.
        extern u64 bit_key;

        /// MP-SPDZ does all comparisons, like in ReLU and MaxPool, in bitlength of 32. Setting this to true will make the protocols do the same.
        extern bool mpspdz_32bit_compaison;

        using FKOS = std::tuple<u8, u64>;
        
        /// methods used by the dealer
        template <typename T>
        T rand() { return prngGlobal.get<T>(); }

        void randomize(shark::span<u128> &share);
        void randomize(shark::span<u64> &share);
        void randomize(shark::span<u8> &share);
        void send_authenticated_ashare(const shark::span<u64> &share);
        void send_authenticated_bshare(const shark::span<u8> &share);
        void send_dcfbit(const shark::span<u64> &share, int bin);
        void send_dcfring(const shark::span<u64> &share, int bin);
        void send_dpfring(const shark::span<u64> &share, int bin);

        /// methods used by the evaluators
        std::pair<shark::span<u128>, shark::span<u128>> recv_authenticated_ashare(u64 size);
        shark::span<FKOS> recv_authenticated_bshare(u64 size);
        shark::span<crypto::DCFBitKey> recv_dcfbit(u64 size, int bin);
        shark::span<crypto::DCFRingKey> recv_dcfring(u64 size, int bin);
        shark::span<crypto::DPFRingKey> recv_dpfring(u64 size, int bin);

        shark::span<u64> authenticated_reconstruct(shark::span<u128> &share, const shark::span<u128> &share_tag);
        shark::span<u8> authenticated_reconstruct(shark::span<FKOS> &share);

        void batch_check();

        FKOS xor_fkos(FKOS x, FKOS y);
        FKOS not_fkos(FKOS x);
    }
}