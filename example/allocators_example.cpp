#include <iostream>
#include "stack_allocator.h"

namespace {

    struct CustomStruct {
        size_t x, y, z;
    };

}

int main() {
    using Allocator = fast_containers::allocators::StackBasedAllocator<::CustomStruct, 1024>;
    return 0;
}
