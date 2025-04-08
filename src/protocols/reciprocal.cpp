#include <shark/protocols/common.hpp>
#include <shark/utils/assert.hpp>
#include <shark/utils/timer.hpp>
#include <shark/protocols/mul.hpp>
#include <shark/protocols/lrs.hpp>
#include <shark/protocols/lut.hpp>
#include <shark/protocols/drelu.hpp>
#include <shark/protocols/select.hpp>

namespace shark
{
    namespace protocols
    {
        namespace reciprocal
        {

            u64 n_m = 7;
            u64 n_e = 4;
            u64 f_p = 20;

            std::vector<u64> make_lut()
            {
                std::vector<u64> lut(1ull << n_m);

                for (u64 i = 0; i < (1ull << n_m); i++)
                {
                    lut[i] = (1ull << f_p) / ((1ull << n_m) + i);
                }
                return lut;
            }

            void gen(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                always_assert(X.size() == Y.size());

                auto d = drelu::call(X);

                shark::span<u64> Xneg(X.size());
                for (u64 i = 0; i < X.size(); i++)
                {
                    Xneg[i] = -X[i];
                }

                send_dcfring(Xneg, 2*f+1);

                shark::span<u64> r_m_scale(X.size());
                shark::span<u64> r_e_scale(X.size());

                randomize(r_m_scale);
                randomize(r_e_scale);

                send_authenticated_ashare(r_m_scale);
                send_authenticated_ashare(r_e_scale);

                auto m_untruncated = mul::call(X, r_m_scale);
                auto mantissa = lrs::call(m_untruncated, 64 - n_m - 1);

                auto mantissa_inverse = lut::call(mantissa, make_lut(), n_m);
                auto inv_untrucated = mul::call(mantissa_inverse, r_e_scale);
                auto inv = lrs::call(inv_untrucated, f_p);
                select::call(d, inv, Y);
            }

            void eval(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                always_assert(X.size() == Y.size());

                shark::span<u64> Xp(X.size());
                
                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    Xp[i] = X[i] - (1ull << (2*f)) - 1;
                }
                
                auto d = drelu::call(Xp);
                
                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    d[i] = d[i] ^ 1;
                }

                auto dcfkeys = recv_dcfring(X.size(), 2*f+1);
                auto [r_m_scale, r_m_scale_tag] = recv_authenticated_ashare(X.size());
                auto [r_e_scale, r_e_scale_tag] = recv_authenticated_ashare(X.size());

                std::vector<u64> knots;
                for (u64 i = 1; i <= (2*f); i++)
                {
                    knots.push_back(1ull << i);
                }
                knots.push_back((1ull << 2*f) + 1);
                u64 m = knots.size();


                shark::span<u128> m_scale_share(X.size());
                shark::span<u128> m_scale_tag(X.size());
                shark::span<u128> e_scale_share(X.size());
                shark::span<u128> e_scale_tag(X.size());

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    m_scale_share[i] = r_m_scale[i];
                    m_scale_tag[i] = r_m_scale_tag[i];
                    e_scale_share[i] = r_e_scale[i];
                    e_scale_tag[i] = r_e_scale_tag[i];

                    auto idx_prev = (-X[i] - 1) % (1ull << (2*f+1));
                    auto t_prev = dcfring_eval(party, dcfkeys[i], idx_prev);
                    auto t_0 = t_prev;

                    for (u64 j = 0; j < m; j++)
                    {
                        auto idx = (knots[j] - X[i] - 1) % (1ull << (2*f+1));
                        auto t = dcfring_eval(party, dcfkeys[i], idx);
                        auto s = std::get<0>(t_prev) - std::get<0>(t);
                        auto s_tag = std::get<1>(t_prev) - std::get<1>(t);

                        if (idx < idx_prev)
                        {
                            s += 1 * party;
                            s_tag += ring_key;
                        }

                        m_scale_share[i] += s * (1ull << (63 - j));
                        m_scale_tag[i] += s_tag * (1ull << (63 - j));

                        e_scale_share[i] += s * (1ull << (2*f - j + n_m));
                        e_scale_tag[i] += s_tag * (1ull << (2*f - j + n_m));

                        t_prev = t;
                        idx_prev = idx;
                    }
                    
                }

                auto m_scale = authenticated_reconstruct(m_scale_share, m_scale_tag);
                auto e_scale = authenticated_reconstruct(e_scale_share, e_scale_tag);

                auto m_untruncated = mul::call(X, m_scale);
                auto mantissa = lrs::call(m_untruncated, 64 - n_m - 1);

                auto mantissa_inverse = lut::call(mantissa, make_lut(), n_m);
                auto inv_untrucated = mul::call(mantissa_inverse, e_scale);
                auto inv = lrs::call(inv_untrucated, f_p);
                select::call(d, inv, Y);

            }

            void call(const shark::span<u64> &X, shark::span<u64> &Y, int f)
            {
                if (party == DEALER)
                {
                    gen(X, Y, f);
                }
                else
                {
                    eval(X, Y, f);
                }
            }

            shark::span<u64> call(const shark::span<u64> &X, int f)
            {
                shark::span<u64> Y(X.size());
                call(X, Y, f);
                return Y;
            }
        }

    }
}
