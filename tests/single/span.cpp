#include <shark/types/span.hpp>
#include <shark/utils/assert.hpp>

struct A
{
    shark::span<int> s;
    A(int n) : s(n) {}
    A() {}
};

int main()
{
    {
        shark::span<A> s(1);
        shark::span<int> a(10);
        s[0].s = a;
    }

    always_assert(shark::span<int>::get_allocs() == 0);
    always_assert(shark::span<A>::get_allocs() == 0);

    return 0;
}