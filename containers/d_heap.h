#ifndef FAST_CONTAINERS_D_HEAP_H
#define FAST_CONTAINERS_D_HEAP_H

#include <iostream>
#include <bit>
#include <vector>
#include <array>
#include <limits>
#include <stdexcept>

#include "utils.h"

namespace fast_containers {
    
    namespace details::d_heap {
        
        inline constexpr std::size_t kDefaultD = 16;
        
    }
    
    template<typename ValueType,
            ValueType DefaultValue,
            std::size_t Capacity,
            std::size_t D,
            auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    class DHeap;

    template<typename ValueType, std::size_t Capacity, std::size_t D = details::d_heap::kDefaultD>
    using MinDHeap = DHeap<ValueType,
            std::numeric_limits<ValueType>::max(),
            Capacity, D,
            [](ValueType parent, ValueType child) -> bool { return (parent < child); }>;

    template<typename ValueType, std::size_t Capacity, std::size_t D = details::d_heap::kDefaultD>
    using MaxDHeap = DHeap<ValueType,
            std::numeric_limits<ValueType>::min(),
            Capacity, D,
            [](ValueType parent, ValueType child) -> bool { return (parent > child); }>;
    
    
    template<typename ValueType,
            ValueType DefaultValue,
            std::size_t Capacity,
            std::size_t D,
            auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    class DHeap {
    public:
        using Reference = ValueType&;

        DHeap();

        ValueType Top();

        void Insert(ValueType element);

        void Pop();
        void Pop(Reference element);

        ~DHeap() = default;

    private:
        void SiftDown(int index);
        void SiftUp(int index);

        bool IsLeaf(std::size_t index);

        constexpr int GetFirstChildIndex(int index);
        constexpr int GetLastChildIndex(int first_child_index);

        static constexpr int GetFirstLeafIndex();
        static constexpr int GetCapacity();

    private:
        std::array<ValueType, GetCapacity()> elements{};
        std::size_t last_element_index{0};

        static constexpr int kDPow = __builtin_ctz(D);
        static constexpr int kFirstLeafIndex = GetFirstLeafIndex();

        static_assert(Capacity >= 2, "Minimum capacity is two");
        static_assert(D >= 2, "Minimum children number is two");
    };


    // Implementation
    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::DHeap() {
        elements.fill(DefaultValue);
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    ValueType DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::Top() {
        return elements[0];
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    void DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::Insert(ValueType element) {
        elements[last_element_index] = element;
        SiftUp(last_element_index++);
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    void DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::Pop() {
        elements[0] = elements[--last_element_index];
        elements[last_element_index] = DefaultValue;
        SiftDown(0);
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    void DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::Pop(ValueType& element) {
        element = elements[0];
        Pop();
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    void DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::SiftDown(int index) {
        while (!IsLeaf(index)) {
            const int first_child_index = GetFirstChildIndex(index);
            const int last_child_index = GetLastChildIndex(first_child_index);

            int child_index = index; // todo with simd
            for (int i = first_child_index; i < last_child_index; i++) {
                if (Comparator(elements[i], elements[child_index])) {
                    child_index = i;
                }
            }

            if (child_index != index) {
                std::swap(elements[index], elements[child_index]);
                index = child_index;
            } else {
                break;
            }
        }
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    void DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::SiftUp(int index) {
        while (index) {
            const int parent_index = (index - 1) >> kDPow;
            if (Comparator(elements[index], elements[parent_index])) {
                std::swap(elements[parent_index], elements[index]);
                index = parent_index;
            } else {
                break;
            }
        }
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    bool DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::IsLeaf(std::size_t index) {
        return index >= kFirstLeafIndex;
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    constexpr int DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::GetFirstChildIndex(int index) {
        return (index << DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::kDPow) + 1;
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    constexpr int DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::GetLastChildIndex(int first_child_index) {
        return first_child_index + D;
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    constexpr int DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::GetFirstLeafIndex() {
        for (int i = 0; i < Capacity; i++) {
            const int first_child_index = (i << __builtin_ctz(D)) + 1;
            if (first_child_index >= Capacity) {
                return i;
            }
        }
        throw std::out_of_range("First leaf index is not found");
    }

    template<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>
    requires fast_containers::utils::IsPowerOfTwo<D>
    constexpr int DHeap<ValueType, DefaultValue, Capacity, D, Comparator>::GetCapacity() {
        const int first_child_index = ((GetFirstLeafIndex() - 1) << __builtin_ctz(D)) + 1;
        return first_child_index + D;
    }

} // End of namespace fast_containers

#endif //FAST_CONTAINERS_D_HEAP_H
