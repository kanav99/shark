#pragma once

#include <tuple>
#include <shark/types/span.hpp>
#include <shark/types/u128.hpp>
#include <shark/utils/assert.hpp>

namespace shark
{
    namespace crypto
    {
        struct DCFRingKey
        {
            shark::span<block> k;
            shark::span<u128> v_ring;
            shark::span<u128> v_tag;
            u128 g_ring;
            u128 g_tag;

            DCFRingKey(const shark::span<block> &k, const shark::span<u128> &v_ring, const shark::span<u128> &v_tag, u128 g_ring, u128 g_tag)
                : k(k), v_ring(v_ring), v_tag(v_tag), g_ring(g_ring), g_tag(g_tag)
            {
                int bin = k.size() - 1;
                always_assert(bin == v_ring.size());
                always_assert(bin == v_tag.size());
            }

            DCFRingKey(shark::span<block> &&k, shark::span<u128> &&v_ring, shark::span<u128> &&v_tag, u128 g_ring, u128 g_tag)
                : k(std::move(k)), v_ring(std::move(v_ring)), v_tag(std::move(v_tag)), g_ring(g_ring), g_tag(g_tag)
            {
                int bin = this->k.size() - 1;
                always_assert(bin == this->v_ring.size());
                always_assert(bin == this->v_tag.size());
            }

            // move constructor
            DCFRingKey(DCFRingKey &&other) : k(std::move(other.k)), v_ring(std::move(other.v_ring)), v_tag(std::move(other.v_tag)), g_ring(other.g_ring), g_tag(other.g_tag)
            {
            }

            // move assignment
            DCFRingKey &operator=(DCFRingKey &&other)
            {
                k = std::move(other.k);
                v_ring = std::move(other.v_ring);
                v_tag = std::move(other.v_tag);
                g_ring = other.g_ring;
                g_tag = other.g_tag;
                return *this;
            }

            DCFRingKey() = default;
        };

        
        std::pair<DCFRingKey, DCFRingKey> dcfring_gen(int bin, const u64 alpha, const bool greaterThan = false);
        std::tuple<u128, u128> dcfring_eval(int party, const DCFRingKey &key, const u64 &x, const bool greaterThan = false);
    }
}