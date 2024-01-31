#include <cassert>

#include "inplace_any.h"

int main() {
    container::any::InplaceTrivialAny<32, alignof(int)> a = 5;
    assert(a.Get<int>() == 5);
    assert(a.Get<int>() != 10);

    a = 543;
    assert(a.Get<int>() != 5);
    assert(a.Get<int>() == 543);

    container::any::InplaceTrivialAny<32, alignof(int)> b{a};
    assert(b.Get<int>() != 5);
    assert(b.Get<int>() == 543);

    b = 20;
    assert(b.Get<int>() == 20);
    assert(b.Get<int>() != 543);
    assert(b.Get<int>() != 5);

    assert(a.Get<int>() != 5);
    assert(a.Get<int>() != 20);
    assert(a.Get<int>() == 543);
    return 0;
}
