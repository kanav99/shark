#pragma once

#include <tuple>
#include <vector>
#include <shark/types/span.hpp>
#include <shark/types/u128.hpp>
#include <shark/utils/assert.hpp>

namespace shark
{
    namespace crypto
    {
        struct DPFRingKey
        {
            shark::span<block> k;
            u128 g_ring;
            u128 g_tag;

            DPFRingKey(const shark::span<block> &k, u128 g_ring, u128 g_tag)
                : k(k), g_ring(g_ring), g_tag(g_tag)
            {
            }

            DPFRingKey(shark::span<block> &&k, u128 g_ring, u128 g_tag)
                : k(std::move(k)), g_ring(g_ring), g_tag(g_tag)
            {
            }

            // move constructor
            DPFRingKey(DPFRingKey &&other) : k(std::move(other.k)), g_ring(other.g_ring), g_tag(other.g_tag)
            {
            }

            // move assignment
            DPFRingKey &operator=(DPFRingKey &&other)
            {
                k = std::move(other.k);
                g_ring = other.g_ring;
                g_tag = other.g_tag;
                return *this;
            }

            DPFRingKey() = default;
        };

        
        std::pair<DPFRingKey, DPFRingKey> dpfring_gen(int bin, const u64 alpha);
        std::tuple<u128, u128> dpfring_evalall_reduce(int party, const DPFRingKey &key, const std::vector<u64> &lut, u64 lut_offset);
    }
}