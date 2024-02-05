#ifndef FAST_CONTAINERS_ID_CONTAINER_H
#define FAST_CONTAINERS_ID_CONTAINER_H

#include <new>
#include <bit>
#include <limits>
#include <type_traits>

namespace fast_containers {

    // First 32 bits - index in IdContainer::buffer_
    // Last 32 bits - IdContainerElementBase::generation_
    using ContainerElementId = uint64_t;

    class IdContainerElementBase;

    namespace details::id_container {

        using Generation = uint64_t;

        // Do not use this element
        class IdContainerEmptyElementCopy {
            Generation generation_{0};
            IdContainerEmptyElementCopy* next_{nullptr};
        };

        template<typename T>
        concept IsStorable = sizeof(T) >= sizeof(IdContainerEmptyElementCopy) &&
                             (alignof(T) % alignof(IdContainerEmptyElementCopy) == 0) &&
                             std::is_nothrow_destructible_v<T> && std::has_single_bit(alignof(T));

        template<typename T>
        concept IsIdContainerElement = std::is_base_of_v<IdContainerElementBase, T>;

    } // End of namespace fast_containers::details::id_container


    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    class IdContainer;
    
    class IdContainerElementBase {
    protected:
        IdContainerElementBase() = default;
        ~IdContainerElementBase() = default;

    public:
        IdContainerElementBase(const IdContainerElementBase&) = delete;
        IdContainerElementBase(IdContainerElementBase&&) = delete;
        IdContainerElementBase& operator=(const IdContainerElementBase&) = delete;
        IdContainerElementBase& operator=(IdContainerElementBase&&) = delete;

    protected:
        template<typename T, std::size_t N>
        requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
        friend class fast_containers::IdContainer;

        details::id_container::Generation generation_{0};
    };
    
    
    namespace details::id_container {

        class IdContainerEmptyElement : protected IdContainerElementBase {
        public:
            using IdContainerElementBase::generation_;

            IdContainerEmptyElement() = default;

            ~IdContainerEmptyElement() = default;

        private:
            template<typename T, std::size_t N>
            requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
            friend class fast_containers::IdContainer;

            IdContainerEmptyElement* next_{nullptr};
        };

        static_assert(sizeof(IdContainerEmptyElementCopy) == sizeof(IdContainerEmptyElement),
                      "Update IdContainerEmptyElementCopy properties");

    } // End of namespace fast_containers::details::id_container

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    class IdContainer {
    private:
        using ElementBase = IdContainerElementBase;
        using EmptyElement = details::id_container::IdContainerEmptyElement;
        using Generation = details::id_container::Generation;

        static constexpr std::size_t kGenerationShift = 32u;

    public:
        using Pointer = T*;

        IdContainer();

        IdContainer(const IdContainer&) = delete;
        IdContainer(IdContainer&&) = delete;
        IdContainer& operator=(const IdContainer&) = delete;
        IdContainer& operator=(IdContainer&&) = delete;

        template<typename... Args>
        ContainerElementId Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>);

        [[nodiscard]] bool Contains(ContainerElementId id);

        Pointer Get(ContainerElementId id);

        void Destroy(ContainerElementId id) noexcept;

        ~IdContainer() = default;

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
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    IdContainer<T, N>::IdContainer() {
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
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    template<typename... Args>
    ContainerElementId IdContainer<T, N>::Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
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
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    bool IdContainer<T, N>::Contains(ContainerElementId id) {
        if (id & GetAlignmentMask()) {
            return false;
        }
        auto expected_generation = GetGeneration(id);
        auto real = GetBase(IdContainer::GetIndexFromId(id));
        return (expected_generation & 1u) && (expected_generation == real->generation_);
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    IdContainer<T, N>::Pointer IdContainer<T, N>::Get(ContainerElementId id) {
        return std::launder(reinterpret_cast<Pointer>(AddressOf(GetIndexFromId(id))));
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    void IdContainer<T, N>::Destroy(ContainerElementId id) noexcept {
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
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    char* IdContainer<T, N>::AddressOf(std::size_t index) {
        return reinterpret_cast<char*>(std::addressof(buffer_)) + index;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    IdContainerElementBase* IdContainer<T, N>::GetBase(std::size_t index) {
        return std::launder(reinterpret_cast<ElementBase*>(AddressOf(index)));
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    std::size_t IdContainer<T, N>::GetIndex(ElementBase* element) {
        return reinterpret_cast<char*>(element) - reinterpret_cast<char*>(std::addressof(buffer_));
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    constexpr ContainerElementId IdContainer<T, N>::GetId(std::size_t index, Generation generation) {
        return (generation << kGenerationShift) | index;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    constexpr std::size_t IdContainer<T, N>::GetIndexFromId(ContainerElementId id) {
        return id & GetIndexMask();
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    constexpr IdContainer<T, N>::Generation IdContainer<T, N>::GetGeneration(ContainerElementId id) {
        return (id & GetGenerationMask()) >> kGenerationShift;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    constexpr ContainerElementId IdContainer<T, N>::GetAlignmentMask() {
        return alignof(T) - 1u;
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    constexpr ContainerElementId IdContainer<T, N>::GetIndexMask() {
        return std::numeric_limits<uint32_t>::max();
    }

    template<typename T, std::size_t N>
    requires details::id_container::IsStorable<T> && details::id_container::IsIdContainerElement<T>
    constexpr ContainerElementId IdContainer<T, N>::GetGenerationMask() {
        return std::numeric_limits<ContainerElementId>::max() ^ GetIndexMask();
    }

} // End of namespace fast_containers

#endif //FAST_CONTAINERS_ID_CONTAINER_H
