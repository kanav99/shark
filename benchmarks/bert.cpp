#include <shark/protocols/init.hpp>
#include <shark/protocols/finalize.hpp>
#include <shark/protocols/input.hpp>
#include <shark/protocols/output.hpp>
#include <shark/protocols/relu.hpp>
#include <shark/protocols/matmul.hpp>
#include <shark/protocols/conv.hpp>
#include <shark/protocols/add.hpp>
#include <shark/protocols/ars.hpp>
#include <shark/protocols/reciprocal.hpp>
#include <shark/protocols/mul.hpp>

#include <shark/utils/timer.hpp>
#include <shark/utils/assert.hpp>

using namespace shark;
using namespace shark::protocols;

int f = 16;

struct BertModel
{

    // config
    u64 n_layers = 12;
    u64 n_heads = 12;
    u64 n_embd = 768;
    u64 n_interm = 3072;
    u64 logdivisor = 3;

    // weights
    std::vector<span<u64>> c_attn_w;
    std::vector<span<u64>> c_attn_b;

    std::vector<span<u64>> c_proj_w;
    std::vector<span<u64>> c_proj_b;

    std::vector<span<u64>> ffn_up_w;
    std::vector<span<u64>> ffn_up_b;

    std::vector<span<u64>> ffn_down_w;
    std::vector<span<u64>> ffn_down_b;

    BertModel() : c_attn_w(n_layers), c_attn_b(n_layers), c_proj_w(n_layers), c_proj_b(n_layers), ffn_up_w(n_layers), ffn_up_b(n_layers), ffn_down_w(n_layers), ffn_down_b(n_layers)
    {
        for (int i = 0; i < n_layers; ++i)
        {
            c_attn_w[i] = span<u64>(n_embd * 3 * n_embd);
            c_attn_b[i] = span<u64>(n_embd * 3);
            c_proj_w[i] = span<u64>(n_embd * n_embd);
            c_proj_b[i] = span<u64>(n_embd);
            ffn_up_w[i] = span<u64>(n_embd * n_interm);
            ffn_up_b[i] = span<u64>(n_interm);
            ffn_down_w[i] = span<u64>(n_interm * n_embd);
            ffn_down_b[i] = span<u64>(n_embd);
        }
    }
};

span<u64> gelu(span<u64> &x)
{
    // return relu::call(x);

    // quad approximation from mpcformer
    //   x^2 / 8 + x/4 + 1/2
    // = 2^f * (x^2 / (2^2f * 8) + x/(2^f * 4) + 1/2)
    // = (x^2 + 2^{f+1}x + 2^{2f+2}) / 2^{f+3}
    auto x2 = mul::call(x, x);

    span<u64> y(x.size());
    for (u64 i = 0; i < x.size(); ++i)
    {
        y[i] = x2[i] + x[i] * (1ull << (f + 1));
        if (party != DEALER)
        {
            y[i] += (1ull << (2*f + 2));
        }
    }
    return ars::call(y, f + 3);
}

span<u64> softmax(u64 a, u64 b, span<u64> &x)
{
    always_assert(x.size() == (a * b));
    auto exps = relu::call(x);

    span<u64> den(a);
    for (u64 i = 0; i < a; ++i)
    {
        u64 sum = 0;
        for (u64 j = 0; j < b; ++j)
        {
            sum += exps[i * b + j];
        }
        den[i] = sum + 1 * (party != DEALER);
    }

    auto den_inv = reciprocal::call(den, f);
    span<u64> den_inv_expanded(a * b);
    for (u64 i = 0; i < a; ++i)
    {
        for (u64 j = 0; j < b; ++j)
        {
            den_inv_expanded[i * b + j] = den_inv[i];
        }
    }

    auto y = mul::call(exps, den_inv_expanded);
    return lrs::call(y, f);
}

span<u64> linear(u64 a, u64 b, u64 c, span<u64> &x, span<u64> &w, span<u64> &bias)
{
    auto u = matmul::call(a, b, c, x, w);
    auto v = add::call(u, bias); // why was `a` a correct argument here, in place of u??
    auto y = ars::call(v, f);
    return y;
}

span<u64> ffn(span<u64> &x, int layer, BertModel &model)
{
    u64 n_token = x.size() / model.n_embd;

    // auto a = matmul::call(n_token, model.n_embd, model.n_interm, x, model.ffn_up_w[layer]);
    // auto b = add::call(a, model.ffn_up_b[layer]);
    // auto c = ars::call(b, 12);
    auto c = linear(n_token, model.n_embd, model.n_interm, x, model.ffn_up_w[layer], model.ffn_up_b[layer]);

    auto d = gelu(c);

    // auto e = matmul::call(n_token, model.n_interm, model.n_embd, d, model.ffn_down_w[layer]);
    // auto f = add::call(e, model.ffn_down_b[layer]);
    // auto g = ars::call(f, 12);
    auto g = linear(n_token, model.n_interm, model.n_embd, d, model.ffn_down_w[layer], model.ffn_down_b[layer]);

    return g;
}

span<u64> layernorm(span<u64> &x, int layer, BertModel &model)
{
    return x;
}

span<u64> view(span<u64> &x, int n_token, int n_peices, int idx)
{
    always_assert(x.size() % (n_token * n_peices) == 0);
    int n_embd = x.size() / n_token;
    span<u64> y(x.size() / n_peices);
    
    for (int i = 0; i < n_token; i++)
    {
        for (int j = 0; j < (n_embd / n_peices); j++)
        {
            y[i * (n_embd / n_peices) + j] = x[i * n_embd + j + idx * (n_embd / n_peices)];
        }
    }

    return y;
}

span<u64> transpose(u64 a, u64 b, span<u64> &x)
{
    always_assert(x.size() == (a * b));
    span<u64> y(x.size());
    for (int i = 0; i < a; i++)
    {
        for (int j = 0; j < b; j++)
        {
            y[j * a + i] = x[i * b + j];
        }
    }

    return y;
}

void concat(span<u64> &x, span<u64> &y, int n_token, int idx)
{
    int x_n_embd = x.size() / n_token;
    int y_n_embd = y.size() / n_token;
    int n_peices = x_n_embd / y_n_embd;

    for (int i = 0; i < n_token; i++)
    {
        for (int j = 0; j < y_n_embd; j++)
        {
            x[i * x_n_embd + j + idx * y_n_embd] = y[i * y_n_embd + j];
        }
    }
}

span<u64> mha(span<u64> &x, int layer, BertModel &model)
{
    u64 n_token = x.size() / model.n_embd;
    // auto a = matmul::call(n_token, model.n_embd, model.n_embd * 3, x, model.c_attn_w[layer]);
    // auto b = add::call(a, model.c_attn_b[layer]);
    // auto c = ars::call(b, 12);
    auto c = linear(n_token, model.n_embd, model.n_embd * 3, x, model.c_attn_w[layer], model.c_attn_b[layer]);

    auto q = view(c, n_token, 3, 0);
    auto k = view(c, n_token, 3, 1);
    auto v = view(c, n_token, 3, 2);

    double logdivisor = model.logdivisor; // divisor = sqrt(n_embd / n_heads);

    span<u64> qks_sm_vs(model.n_embd * n_token);
    for (int i = 0; i < model.n_heads; i++)
    {
        auto qi = view(q, n_token, model.n_heads, i);
        auto ki = view(k, n_token, model.n_heads, i);
        auto vi = view(v, n_token, model.n_heads, i);

        auto kt = transpose(n_token, model.n_embd / model.n_heads, ki);
        auto qk = matmul::call(n_token, model.n_embd / model.n_heads, n_token, qi, kt);
        auto s = ars::call(qk, f + logdivisor);

        auto qks_sm = softmax(n_token, n_token, s);

        auto qks_sm_v = matmul::call(n_token, n_token, model.n_embd / model.n_heads, qks_sm, vi);
        concat(qks_sm_vs, qks_sm_v, n_token, i);
    }
    auto z = ars::call(qks_sm_vs, f);

    // auto d = matmul::call(n_token, model.n_embd, model.n_embd, z, model.c_proj_w[layer]);
    // auto e = add::call(d, model.c_proj_b[layer]);
    // auto f = ars::call(e, 12);
    auto f = linear(n_token, model.n_embd, model.n_embd, z, model.c_proj_w[layer], model.c_proj_b[layer]);

    return f;
}

span<u64> layer(span<u64> &x, int layer, BertModel &model)
{
    auto a = layernorm(x, layer, model);
    auto b = mha(a, layer, model);
    auto c = add::call(a, b);

    auto d = layernorm(c, layer, model);
    auto e = ffn(d, layer, model);
    auto f = add::call(d, e);

    return f;
}

span<u64> inference(span<u64> &x, BertModel &model)
{
    span<u64> y = x;
    for (int i = 0; i < model.n_layers; i++)
    {
        y = layer(y, i, model);
    }

    return y;
}

void model_input(BertModel &model)
{
    for (int i = 0; i < model.n_layers; i++)
    {
        input::call(model.c_attn_w[i], SERVER);
        input::call(model.c_attn_b[i], SERVER);
        input::call(model.c_proj_w[i], SERVER);
        input::call(model.c_proj_b[i], SERVER);
        input::call(model.ffn_up_w[i], SERVER);
        input::call(model.ffn_up_b[i], SERVER);
        input::call(model.ffn_down_w[i], SERVER);
        input::call(model.ffn_down_b[i], SERVER);
    }
}

int main(int argc, char **argv)
{
    init::from_args(argc, argv);
    
    BertModel model;

    u64 n_token = 128;
    span<u64> x(n_token * model.n_embd);

    utils::start_timer("input");
    input::call(x, CLIENT);
    model_input(model);
    utils::stop_timer("input");

    if (party != DEALER)
        peer->sync();

    utils::start_timer("bert");
    auto y = inference(x, model);
    output::call(y);
    utils::stop_timer("bert");

    finalize::call();
    utils::print_all_timers();
}
