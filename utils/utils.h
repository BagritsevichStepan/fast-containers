#ifndef FAST_CONTAINERS_UTILS_H
#define FAST_CONTAINERS_UTILS_H

#include <bit>

namespace fast_containers::utils {

    template<auto Number>
    concept IsPowerOfTwo = std::has_single_bit(size_t(Number));

}

#endif //FAST_CONTAINERS_UTILS_H
