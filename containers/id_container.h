#ifndef FAST_CONTAINERS_ID_CONTAINER_H
#define FAST_CONTAINERS_ID_CONTAINER_H

#include <new>
#include <bit>
#include <type_traits>

#include "common.h"

namespace fast_containers {

    class DefaultIdContainerTag;

    // First 32 bits - index in IdContainer::buffer_
    // Second 32 bits - IdContainerElementBase::generation_
    using ContainerElementId = uint64_t;

    template<typename Tag = DefaultIdContainerTag>
    class IdContainerElement;

    namespace details::id_container {

        static_assert(sizeof(size_t) == 8);

        using Generation = uint32_t;

        // Do not use this element
        class IdContainerEmptyElementCopy {
            Generation generation_{0};
            IdContainerEmptyElementCopy* next_{nullptr};
        };

        template<typename T>
        concept IsStorable = sizeof(T) >= sizeof(IdContainerEmptyElementCopy) &&
                             (alignof(T) % alignof(IdContainerEmptyElementCopy) == 0) &&
                             std::is_nothrow_destructible_v<T> && std::has_single_bit(alignof(T));

        template<typename T, typename Tag>
        concept IsIdContainerElement = std::is_base_of_v<IdContainerElement<Tag>, T>;

    } // End of namespace fast_containers::details::id_container

    template<typename T, ContainerCapacity N, typename Tag = DefaultIdContainerTag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    class IdContainer;

    namespace details::id_container {

        class IdContainerElementBase {
        protected:
            IdContainerElementBase() = default;
            ~IdContainerElementBase() = default;

        public:
            IdContainerElementBase(const IdContainerElementBase&) = delete;
            IdContainerElementBase(IdContainerElementBase&&) = delete;
            IdContainerElementBase& operator=(const IdContainerElementBase&) = delete;
            IdContainerElementBase& operator=(IdContainerElementBase&&) = delete;

        public:
            template<typename T, ContainerCapacity N, typename Tag>
            requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
            friend class IdContainer;

            Generation generation_{0};
        };

        class IdContainerEmptyElement : protected IdContainerElementBase {
        public:
            using IdContainerElementBase::generation_;

            IdContainerEmptyElement() = default;

            ~IdContainerEmptyElement() = default;

        public:
            template<typename T, ContainerCapacity N, typename Tag>
            requires IsStorable<T> && IsIdContainerElement<T, Tag>
            friend class IdContainer;

            IdContainerEmptyElement* next_{nullptr};
        };

        static_assert(sizeof(IdContainerEmptyElementCopy) == sizeof(IdContainerEmptyElement),
                      "Update IdContainerEmptyElementCopy properties");

    } // End of namespace fast_containers::details::id_container


    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    class IdContainer {
    private:
        using Element = IdContainerElement<Tag>;
        using ElementBase = details::id_container::IdContainerElementBase;
        using EmptyElement = details::id_container::IdContainerEmptyElement;
        using Generation = details::id_container::Generation;

    public:
        using Pointer = T*;
        using ConstPointer = const T*;

        IdContainer() = default;

        IdContainer(const IdContainer&) = delete;
        IdContainer(IdContainer&&) = delete;
        IdContainer& operator=(const IdContainer&) = delete;
        IdContainer& operator=(IdContainer&&) = delete;

        template<typename... Args>
        ContainerElementId Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);

        [[nodiscard]] bool Contains(ContainerElementId id) const;

        Pointer Get(ContainerElementId id);
        ConstPointer Get(ContainerElementId id) const;

        void Destroy(ContainerElementId id) noexcept;

        ~IdContainer() = default;

    private:
        ElementBase* GetBase(ContainerCapacity index);

        static ContainerElementId GetId(ContainerCapacity index, Generation generation);
        static ContainerCapacity GetIndex(ContainerElementId id);
        static Generation GetGeneration(ContainerElementId id);

        static constexpr ContainerElementId GetIndexMask();
        static constexpr ContainerElementId GetAlignmentMask();

        static constexpr ContainerElementId GetGenerationMask();

    private:
        std::aligned_storage_t<sizeof(T) * N, alignof(T)> buffer_{};
        EmptyElement* head_{nullptr};
        EmptyElement* tail_{nullptr};

        static_assert(sizeof(T) * N <= size_t(std::numeric_limits<uint32_t>::max()), "Too much memory is allocated");
    };


    // Implementation

    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    template<typename... Args>
    ContainerElementId IdContainer<T, N, Tag>::Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
        ContainerCapacity index;
        details::id_container::Generation next_generation;

        {
            auto head = head_;

            //todo index
            next_generation = head_->generation_ + 1;

            head_ = head_->next_;
            delete head;
        }

        auto element = new (std::addressof(buffer_[index])) T(std::forward<Args>(args)...);
        element->generation_ = next_generation;
        return GetId(index, next_generation);
    }

    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    bool IdContainer<T, N, Tag>::Contains(ContainerElementId id) const {
        if (id & GetAlignmentMask()) {
            return false;
        }
        auto expected_generation = GetGeneration(id);
        return (expected_generation & 1) && (expected_generation == GetBase(GetIndex(id))->generation_);
    }

    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    IdContainer<T, N, Tag>::Pointer IdContainer<T, N, Tag>::Get(ContainerElementId id) {
        return std::launder(reinterpret_cast<Pointer>(std::addressof(buffer_[GetIndex(id)])));
    }

    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    IdContainer<T, N, Tag>::ConstPointer IdContainer<T, N, Tag>::Get(ContainerElementId id) const {
        return std::launder(reinterpret_cast<ConstPointer>(std::addressof(buffer_[GetIndex(id)])));
    }

    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>void
    IdContainer<T, N, Tag>::Destroy(ContainerElementId id) noexcept {
        ContainerCapacity index = GetIndex(id);
        Pointer element = Get(id);
        Generation next_generation = element->generation_ + 1;

        element->~T();

        auto empty_element = new (std::addressof(buffer_[index])) EmptyElement();
        empty_element->generation_ = next_generation;
        tail_->next_ = empty_element;
        tail_ = empty_element;
    }


    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    details::id_container::IdContainerElementBase* IdContainer<T, N, Tag>::GetBase(ContainerCapacity index) {
        return std::launder(reinterpret_cast<details::id_container::IdContainerElementBase*>(std::addressof(buffer_[index])));
    }

    template<typename T, ContainerCapacity N, typename Tag>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T, Tag>
    constexpr ContainerElementId IdContainer<T, N, Tag>::GetAlignmentMask() {
        return alignof(T) - 1;
    }

} // End of namespace fast_containers

#endif //FAST_CONTAINERS_ID_CONTAINER_H
