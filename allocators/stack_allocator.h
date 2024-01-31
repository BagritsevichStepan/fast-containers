#ifndef FAST_CONTAINERS_STACK_ALLOCATOR_H
#define FAST_CONTAINERS_STACK_ALLOCATOR_H

#include <bit>
#include <type_traits>
#include "common.h"

namespace fast_containers::allocators {

    namespace details::chunk_stack_based_allocator {

        struct Node {
            Node* next_{nullptr};
        };

        template<typename T>
        concept IsStorable = sizeof(T) >= sizeof(Node) && (alignof(T) % alignof(Node) == 0);
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN = 1024>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    class StackBasedAllocator;

    namespace details::chunk_stack_based_allocator {

        template<typename T>
        class ChunkStackBasedAllocator {
        public:
            ChunkStackBasedAllocator() = default;

            template<typename Storage>
            ChunkStackBasedAllocator(Storage* storage, ContainerCapacity index, ChunkCapacity chunk_capacity, size_t chunks_count);

            ChunkStackBasedAllocator(ChunkStackBasedAllocator&& other) noexcept;
            ChunkStackBasedAllocator& operator=(ChunkStackBasedAllocator&& other) noexcept;

            void Clear();

            template<typename _T, ContainerCapacity TotalN, size_t MaxN>
            requires details::chunk_stack_based_allocator::IsStorable<_T>
            friend class fast_containers::allocators::StackBasedAllocator;

        public:
            using Pointer = T*;

            ChunkStackBasedAllocator(const ChunkStackBasedAllocator&) = delete;
            ChunkStackBasedAllocator& operator=(const ChunkStackBasedAllocator&) = delete;

            Pointer Allocate();
            void Deallocate(Pointer pointer);

            ~ChunkStackBasedAllocator();

        private:
            Node* list_head_{nullptr};
        };

    } // End of namespace fast_containers::allocators::details::chunk_stack_based_allocator


    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    class StackBasedAllocator {
    private:
        using Node = details::chunk_stack_based_allocator::Node;
        using ChunkAllocator = details::chunk_stack_based_allocator::ChunkStackBasedAllocator<T>;

    public:
        using value_type = T;
        using pointer = T*;
        using propagate_on_container_move_assignment = std::true_type;

        StackBasedAllocator();

        StackBasedAllocator(const StackBasedAllocator&) = delete;
        StackBasedAllocator(StackBasedAllocator&&) = delete;
        StackBasedAllocator& operator=(const StackBasedAllocator&) = delete;
        StackBasedAllocator& operator=(StackBasedAllocator&& other) = delete;

        pointer allocate(size_t n);
        void deallocate(pointer pointer, size_t n);

        ~StackBasedAllocator();

    private:
        void Clear();

        size_t GetChunkAllocatorIndex(size_t n);

        static constexpr size_t GetBufferSize();
        static constexpr size_t GetChunkAllocatorsNumber();

    private:
        std::aligned_storage_t<GetBufferSize(), alignof(T)> buffer_{};
        std::array<ChunkAllocator, GetChunkAllocatorsNumber()> chunk_allocators_;

    };


    //Implementation

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    StackBasedAllocator<T, TotalN, MaxN>::StackBasedAllocator() {
        size_t index = 0;
        for (int i = 0; i < GetChunkAllocatorsNumber(); i++) {
            size_t chunk_capacity = 1u << i;
            size_t chunks_count = (TotalN / chunk_capacity) + 1;

            chunk_allocators_[i] = ChunkAllocator(&buffer_, index, chunk_capacity, chunks_count);

            index += chunks_count * chunk_capacity * sizeof(T);
        }
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    StackBasedAllocator<T, TotalN, MaxN>::pointer StackBasedAllocator<T, TotalN, MaxN>::allocate(size_t n) {
        return chunk_allocators_[GetChunkAllocatorIndex(n)].Allocate();
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    void StackBasedAllocator<T, TotalN, MaxN>::deallocate(StackBasedAllocator::pointer pointer, size_t n) {
        return chunk_allocators_[GetChunkAllocatorIndex(n)].Deallocate(pointer);
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    StackBasedAllocator<T, TotalN, MaxN>::~StackBasedAllocator() {
        Clear();
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    void StackBasedAllocator<T, TotalN, MaxN>::Clear() {
        for (int i = 0; i < GetChunkAllocatorsNumber(); i++) {
            //chunk_allocators_[i].Clear();
        }
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>size_t
    StackBasedAllocator<T, TotalN, MaxN>::GetChunkAllocatorIndex(size_t n) {
        return __builtin_ctz(std::bit_ceil(n));
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    constexpr size_t StackBasedAllocator<T, TotalN, MaxN>::GetBufferSize() {
        size_t chunks_number = GetChunkAllocatorsNumber();
        size_t result = 0;
        for (size_t i = 0; i < chunks_number; i++) {
            size_t chunk_capacity = 1u << i;
            size_t chunks_count = (TotalN / chunk_capacity) + 1;
            result += chunks_count * chunk_capacity * sizeof(T);
        }
        return result;
    }

    template<typename T, ContainerCapacity TotalN, size_t MaxN>
    requires details::chunk_stack_based_allocator::IsStorable<T>
    constexpr size_t StackBasedAllocator<T, TotalN, MaxN>::GetChunkAllocatorsNumber() {
        return __builtin_ctz(std::bit_ceil(MaxN));
    }


    //ChunkStackBasedAllocator implementation
    namespace details::chunk_stack_based_allocator {

        template<typename T>
        template<typename Storage>
        ChunkStackBasedAllocator<T>::ChunkStackBasedAllocator(Storage* storage, ContainerCapacity index,
                                                              ChunkCapacity chunk_capacity, size_t chunks_count) {
            ContainerCapacity current_index = index;
            for (int i = 0; i < chunks_count; i++) {
                auto pointer = reinterpret_cast<char*>(storage) + current_index;
                auto ptr = new (pointer) Node();
                ptr->next_ = list_head_;
                list_head_ = ptr;
                current_index += chunk_capacity * sizeof(T);
            }
        }


        template<typename T>
        ChunkStackBasedAllocator<T>::ChunkStackBasedAllocator(ChunkStackBasedAllocator&& other) noexcept {
            std::swap(list_head_, other.list_head_);
        }

        template<typename T>
        ChunkStackBasedAllocator<T>& ChunkStackBasedAllocator<T>::operator=(ChunkStackBasedAllocator&& other) noexcept {
            if (this != &other) {
                std::swap(list_head_, other.list_head_);
            }
            return *this;
        }

        template<typename T>
        ChunkStackBasedAllocator<T>::Pointer ChunkStackBasedAllocator<T>::Allocate() {
            Node* ptr = list_head_;
            list_head_ = list_head_->next_;
            return reinterpret_cast<Pointer>(ptr);
        }

        template<typename T>
        void ChunkStackBasedAllocator<T>::Deallocate(ChunkStackBasedAllocator::Pointer pointer) {
            Node* node_ptr = reinterpret_cast<Node*>(pointer);
            node_ptr->next_ = list_head_;
            list_head_ = node_ptr;
        }

        template<typename T>
        ChunkStackBasedAllocator<T>::~ChunkStackBasedAllocator() {
            //Clear();
        }

        template<typename T>
        void ChunkStackBasedAllocator<T>::Clear() {
            auto node_ptr = list_head_;
            while (node_ptr) {
                auto tmp_ptr = node_ptr;
                node_ptr = node_ptr->next_;
                delete tmp_ptr;
            }

            list_head_ = nullptr;
        }

    }

} // End of namespace fast_containers::allocators

#endif //FAST_CONTAINERS_STACK_ALLOCATOR_H
