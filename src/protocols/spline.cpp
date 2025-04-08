
#include <shark/protocols/common.hpp>
#include <shark/protocols/spline.hpp>

namespace shark
{
    namespace protocols
    {
        namespace spline
        {
            u64 nCr(u64 n, u64 r)
            {
                if (r > n)
                    return 0;
                if (r == 0 || r == n)
                    return 1;
                return nCr(n - 1, r - 1) + nCr(n - 1, r);
            }

            u64 pow(u64 x, int p)
            {
                if (p < 0)
                    return u64(0);
                if (p == 0)
                    return u64(1);
                u64 res = pow(x, p / 2);
                res = res * res;
                if (p % 2)
                    res = res * x;
                return res;
            }

            void gen(int bin, int degree, const shark::span<u64> &X, shark::span<u64> &Y)
            {
                always_assert(X.size() == Y.size());

                shark::span<u64> Xneg(X.size());
                for (u64 i = 0; i < X.size(); i++)
                {
                    Xneg[i] = -X[i];
                }

                send_dcfring(Xneg, bin);

                shark::span<u64> r_selected_poly((degree + 1) * X.size());
                randomize(r_selected_poly);
                send_authenticated_ashare(r_selected_poly);

                shark::span<u64> S(X.size() * (degree + 1));
                shark::span<u64> T(X.size() * (degree + 1));
                randomize(Y);

                for (u64 i = 0; i < X.size(); ++i)
                {
                    for (u64 k = 0; k < degree + 1; ++k)
                    {
                        S[i * (degree + 1) + k] = pow(-X[i], k);
                    }
                }

                for (u64 i = 0; i < X.size(); ++i)
                {
                    for (u64 k = 0; k < degree + 1; ++k)
                    {
                        u64 coeff = 0;
                        for (u64 j = k; j < degree + 1; ++j)
                        {
                            coeff = coeff - nCr(j, k) * r_selected_poly[i * (degree + 1) + j] * pow(-X[i], j - k);
                        }
                        T[i * (degree + 1) + k] = coeff;
                    }

                    T[i * (degree + 1)] = T[i * (degree + 1)] + Y[i];
                }

                send_authenticated_ashare(S);
                send_authenticated_ashare(T);

            }

            void eval(int bin, int degree, const std::vector<u64> &knots, const std::vector<u64> &polynomials, const shark::span<u64> &X, shark::span<u64> &Y)
            {
                u64 m = knots.size();
                u64 p = polynomials.size();
                always_assert((m + 1) * (degree + 1) == p);
                always_assert(X.size() == Y.size());
                always_assert(bin <= 64);

                // check if knots are sorted
                for (u64 i = 1; i < m; i++)
                {
                    always_assert(knots[i - 1] < knots[i]);
                }
                if (bin != 64)
                    always_assert(knots[m - 1] < (1ull << bin));

                auto dcfkeys = recv_dcfring(X.size(), bin);
                auto [r_selected_poly, r_selected_poly_tag] = recv_authenticated_ashare((degree + 1) * X.size());
                auto [S_share, S_tag] = recv_authenticated_ashare(X.size() * (degree + 1));
                auto [T_share, T_tag] = recv_authenticated_ashare(X.size() * (degree + 1));

                shark::span<u128> selected_poly_share((degree + 1) * X.size());
                shark::span<u128> selected_poly_tag((degree + 1) * X.size());
                for (u64 i = 0; i < (degree + 1) * X.size(); ++i)
                {
                    selected_poly_share[i] = r_selected_poly[i];
                    selected_poly_tag[i] = r_selected_poly_tag[i];
                }

                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    auto idx_prev = (-X[i] - 1);
                    if (bin != 64) 
                        idx_prev = idx_prev % (1ull << bin);
                    auto t_prev = dcfring_eval(party, dcfkeys[i], idx_prev);
                    auto t_0 = t_prev;
                    for (u64 j = 0; j < m + 1; j++)
                    {
                        std::tuple<u128, u128> t;
                        auto idx = ((j == m ? 0 : knots[j]) - X[i] - 1);
                        if (bin != 64) 
                            idx = idx % (1ull << bin);
                        if (j == m)
                            t = t_0;
                        else
                            t = dcfring_eval(party, dcfkeys[i], idx);
                        auto s = std::get<0>(t_prev) - std::get<0>(t);
                        auto s_tag = std::get<1>(t_prev) - std::get<1>(t);

                        if (idx < idx_prev)
                        {
                            s += 1 * party;
                            s_tag += ring_key;
                        }

                        for (u64 k = 0; k < degree + 1; k++)
                        {
                            selected_poly_share[i * (degree + 1) + k] += s * polynomials[j * (degree + 1) + k];
                            selected_poly_tag[i * (degree + 1) + k] += s_tag * polynomials[j * (degree + 1) + k];
                        }

                        t_prev = t;
                        idx_prev = idx;

                    }
                    
                }

                auto selected_poly = authenticated_reconstruct(selected_poly_share, selected_poly_tag);

                shark::span<u128> Y_share(X.size());
                shark::span<u128> Y_tag(X.size());
                #pragma omp parallel for
                for (u64 i = 0; i < X.size(); i++)
                {
                    u128 y_share = 0;
                    u128 y_tag = 0;

                    for (u64 k = 0; k < degree + 1; ++k)
                    {
                        y_share += T_share[i * (degree + 1) + k] * pow(X[i], k);
                        y_tag += T_tag[i * (degree + 1) + k] * pow(X[i], k);

                        for (u64 j = 0; j < k + 1; j++)
                        {
                            y_share += nCr(k, j) * S_share[i * (degree + 1) + k - j] * selected_poly[i * (degree + 1) + k] * pow(X[i], j);
                            y_tag += nCr(k, j) * S_tag[i * (degree + 1) + k - j] * selected_poly[i * (degree + 1) + k] * pow(X[i], j);
                        }
                    }

                    Y_share[i] = y_share;
                    Y_tag[i] = y_tag;

                }

                Y = authenticated_reconstruct(Y_share, Y_tag);

            }

            void call(int bin, int degree, const std::vector<u64> &knots, const std::vector<u64> &polynomials, const shark::span<u64> &X, shark::span<u64> &Y)
            {
                if (party == DEALER)
                {
                    gen(bin, degree, X, Y);
                }
                else
                {
                    eval(bin, degree, knots, polynomials, X, Y);
                }
            }

            shark::span<u64> call(int bin, int degree, const std::vector<u64> &knots, const std::vector<u64> &polynomials, const shark::span<u64> &X)
            {
                shark::span<u64> Y(X.size());
                call(bin, degree, knots, polynomials, X, Y);
                return Y;
            }
        }
    }
}
