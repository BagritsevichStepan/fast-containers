#include <iostream>
#include <bit>

#include "d_heap.h"

int main() {
    const std::size_t capacity = 18;

    int arr[capacity];

    for (int &i : arr) {
        i = rand() % 20;
        std::cout << i << " ";
    }
    std::cout << std::endl;

    fast_containers::MinDHeap<std::int32_t, capacity> min_heap{};
    fast_containers::MaxDHeap<std::int32_t, capacity> max_heap{};

    for (int i : arr) {
        min_heap.Insert(i);
        max_heap.Insert(i);
    }

    std::cout << "MinDHeap: ";
    for (int i = 0; i < capacity; i++) {
        std::cout << min_heap.Top() << " ";
        min_heap.Pop();
    }
    std::cout << std::endl;

    std::cout << "MaxDHeap: ";
    for (int i = 0; i < capacity; i++) {
        std::cout << max_heap.Top() << " ";
        max_heap.Pop();
    }
    std::cout << std::endl;
    return 0;
}