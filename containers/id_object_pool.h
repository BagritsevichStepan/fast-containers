#ifndef FAST_CONTAINERS_ID_OBJECT_POOL_H
#define FAST_CONTAINERS_ID_OBJECT_POOL_H

#include <new>
#include <bit>
#include <limits>
#include <type_traits>

namespace fast_containers {

    // First 32 bits - index in IdObjectPool::buffer_
    // Last 32 bits - IdObjectPoolElementBase::generation_
    using ContainerElementId = uint64_t;

    class IdObjectPoolElementBase;

    namespace details::id_container {

        using Generation = uint64_t;

        // Do not use this element
        class IdObjectPoolEmptyElementCopy {
            Generation generation_{0};
            IdObjectPoolEmptyElementCopy* next_{nullptr};
        };

        template<typename T>
        concept IsStorable = sizeof(T) >= sizeof(IdObjectPoolEmptyElementCopy) &&
                             (alignof(T) % alignof(IdObjectPoolEmptyElementCopy) == 0) &&
                             std::is_nothrow_destructible_v<T> && std::has_single_bit(alignof(T));

        template<typename T>
        concept IsIdObjectPoolElement = std::is_base_of_v<IdObjectPoolElementBase, T>;

    } // End of namespace fast_containers::details::id_container


    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    class IdObjectPool;
    
    class IdObjectPoolElementBase {
    protected:
        IdObjectPoolElementBase() = default;
        ~IdObjectPoolElementBase() = default;

    public:
        IdObjectPoolElementBase(const IdObjectPoolElementBase&) = delete;
        IdObjectPoolElementBase(IdObjectPoolElementBase&&) = delete;
        IdObjectPoolElementBase& operator=(const IdObjectPoolElementBase&) = delete;
        IdObjectPoolElementBase& operator=(IdObjectPoolElementBase&&) = delete;

    protected:
        template<typename T, std::size_t N>
        requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
        friend class fast_containers::IdObjectPool;

        details::id_container::Generation generation_{0};
    };
    
    
    namespace details::id_container {

        class IdObjectPoolEmptyElement : protected IdObjectPoolElementBase {
        public:
            using IdObjectPoolElementBase::generation_;

            IdObjectPoolEmptyElement() = default;

            ~IdObjectPoolEmptyElement() = default;

        private:
            template<typename T, std::size_t N>
            requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
            friend class fast_containers::IdObjectPool;

            IdObjectPoolEmptyElement* next_{nullptr};
        };

        static_assert(sizeof(IdObjectPoolEmptyElementCopy) == sizeof(IdObjectPoolEmptyElement),
                      "Update IdObjectPoolEmptyElementCopy properties");

    } // End of namespace fast_containers::details::id_container

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    class IdObjectPool {
    private:
        using ElementBase = IdObjectPoolElementBase;
        using EmptyElement = details::id_container::IdObjectPoolEmptyElement;
        using Generation = details::id_container::Generation;

        static constexpr std::size_t kGenerationShift = 32u;

    public:
        using Pointer = T*;

        IdObjectPool();

        IdObjectPool(const IdObjectPool&) = delete;
        IdObjectPool(IdObjectPool&&) = delete;
        IdObjectPool& operator=(const IdObjectPool&) = delete;
        IdObjectPool& operator=(IdObjectPool&&) = delete;

        template<typename... Args>
        ContainerElementId Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);

        [[nodiscard]] bool Contains(ContainerElementId id);

        Pointer Get(ContainerElementId id);

        void Destroy(ContainerElementId id) noexcept;

        ~IdObjectPool() = default;

    private:
        [[nodiscard]] char* AddressOf(std::size_t index);
        [[nodiscard]] ElementBase* GetBase(std::size_t index);
        std::size_t GetIndex(ElementBase* element);

        static constexpr ContainerElementId GetId(std::size_t index, Generation generation);
        static constexpr std::size_t GetIndexFromId(ContainerElementId id);
        static constexpr Generation GetGeneration(ContainerElementId id);

        static constexpr ContainerElementId GetAlignmentMask();
        static constexpr ContainerElementId GetIndexMask();
        static constexpr ContainerElementId GetGenerationMask();

    private:
        std::aligned_storage_t<sizeof(T) * (N + 1), alignof(T)> buffer_{};
        EmptyElement* head_{nullptr};
        EmptyElement* tail_{nullptr};

        static_assert(sizeof(T) * (N + 1) <= std::numeric_limits<uint32_t>::max(), "Too much memory is allocated");
    };


    // Implementation
    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    IdObjectPool<T, N>::IdObjectPool() {
        for (int i = 0; i <= sizeof(T) * N; i += sizeof(T)) {
            auto empty_element = new(AddressOf(i)) EmptyElement();
            if (i) {
                tail_->next_ = empty_element;
                tail_ = empty_element;
            } else {
                head_ = tail_ = empty_element;
            }
        }
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    template<typename... Args>
    ContainerElementId IdObjectPool<T, N>::Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        std::size_t index;
        details::id_container::Generation next_generation;

        {
            auto head = head_;

            index = GetIndex(head);
            next_generation = head_->generation_ + 1u;

            head_ = head_->next_;
            head->~EmptyElement();
        }

        auto element = new (AddressOf(index)) T(std::forward<Args>(args)...);
        element->generation_ = next_generation;
        return GetId(index, next_generation);
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    bool IdObjectPool<T, N>::Contains(ContainerElementId id) {
        if (id & GetAlignmentMask()) {
            return false;
        }
        auto expected_generation = GetGeneration(id);
        auto real = GetBase(IdObjectPool::GetIndexFromId(id));
        return (expected_generation & 1u) && (expected_generation == real->generation_);
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    IdObjectPool<T, N>::Pointer IdObjectPool<T, N>::Get(ContainerElementId id) {
        return std::launder(reinterpret_cast<Pointer>(AddressOf(GetIndexFromId(id))));
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    void IdObjectPool<T, N>::Destroy(ContainerElementId id) noexcept {
        std::size_t index;
        Generation next_generation;

        {
            Pointer element = Get(id);

            index = GetIndexFromId(id);
            next_generation = element->generation_ + 1u;

            element->~T();
        }

        auto empty_element = new(AddressOf(index)) EmptyElement();
        empty_element->generation_ = next_generation;
        tail_->next_ = empty_element;
        tail_ = empty_element;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    char* IdObjectPool<T, N>::AddressOf(std::size_t index) {
        return reinterpret_cast<char*>(std::addressof(buffer_)) + index;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    IdObjectPoolElementBase* IdObjectPool<T, N>::GetBase(std::size_t index) {
        return std::launder(reinterpret_cast<ElementBase*>(AddressOf(index)));
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    std::size_t IdObjectPool<T, N>::GetIndex(ElementBase* element) {
        return reinterpret_cast<char*>(element) - reinterpret_cast<char*>(std::addressof(buffer_));
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    constexpr ContainerElementId IdObjectPool<T, N>::GetId(std::size_t index, Generation generation) {
        return (generation << kGenerationShift) | index;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    constexpr std::size_t IdObjectPool<T, N>::GetIndexFromId(ContainerElementId id) {
        return id & GetIndexMask();
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    constexpr IdObjectPool<T, N>::Generation IdObjectPool<T, N>::GetGeneration(ContainerElementId id) {
        return (id & GetGenerationMask()) >> kGenerationShift;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    constexpr ContainerElementId IdObjectPool<T, N>::GetAlignmentMask() {
        return alignof(T) - 1u;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    constexpr ContainerElementId IdObjectPool<T, N>::GetIndexMask() {
        return std::numeric_limits<uint32_t>::max();
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdObjectPoolElement<T>
    constexpr ContainerElementId IdObjectPool<T, N>::GetGenerationMask() {
        return std::numeric_limits<ContainerElementId>::max() ^ GetIndexMask();
    }

} // End of namespace fast_containers

#endif //FAST_CONTAINERS_ID_OBJECT_POOL_H
