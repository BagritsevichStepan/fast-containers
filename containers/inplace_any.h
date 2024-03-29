#ifndef FAST_CONTAINERS_INPLACE_ANY_H
#define FAST_CONTAINERS_INPLACE_ANY_H

#include <utility>
#include <type_traits>
#include <cstring>

namespace fast_containers {

    // todo inplace base



    template<std::size_t N, std::size_t Alignment = N>
    class InplaceAny {

    private:
        std::aligned_storage_t<N, Alignment> buffer_;
    };


    template<std::size_t N, std::size_t Alignment = N>
    class InplaceTrivialAny {
    private:
        template<typename T>
        using NonConstT = std::remove_cv_t<std::remove_reference_t<T>>;

        template<typename T>
        static constexpr bool IsCopyable = std::is_trivially_copyable_v<NonConstT<T>> && (sizeof(T) <= N);

    public:
        InplaceTrivialAny() = default;

        InplaceTrivialAny(const InplaceTrivialAny& other);
        InplaceTrivialAny& operator=(const InplaceTrivialAny& other);

        template<typename T, typename = std::enable_if_t<IsCopyable<T>>>
        InplaceTrivialAny(T&& other) noexcept;

        template<typename T, typename = std::enable_if_t<IsCopyable<T>>>
        InplaceTrivialAny& operator=(T&& other) noexcept;

        template<typename T>
        T& Get();

        template<typename T>
        const T& Get() const;

        // todo destroy
        ~InplaceTrivialAny() = default;

        static constexpr std::size_t GetCapacity() noexcept;

    private:
        template<typename T>
        void Copy(T&& other);

        std::aligned_storage_t<N, Alignment> buffer_;
    };


    // Implementation

    // InplaceTrivialAny
    template<std::size_t N, std::size_t Alignment>
    InplaceTrivialAny<N, Alignment>::InplaceTrivialAny(const InplaceTrivialAny& other) {
        Copy(other.buffer_);
    }

    template<std::size_t N, std::size_t Alignment>
    InplaceTrivialAny<N, Alignment> &InplaceTrivialAny<N, Alignment>::operator=(const InplaceTrivialAny& other) {
        if (this != &other) {
            Copy(other.buffer_);
        }
        return *this;
    }

    template<std::size_t N, std::size_t Alignment>
    template<typename T, typename>
    InplaceTrivialAny<N, Alignment>::InplaceTrivialAny(T&& other) noexcept {
        Copy(std::forward<T>(other));
    }

    template<std::size_t N, std::size_t Alignment>
    template<typename T, typename>
    InplaceTrivialAny<N, Alignment>& InplaceTrivialAny<N, Alignment>::operator=(T&& other) noexcept {
        Copy(std::forward<T>(other));
        return *this;
    }

    template<std::size_t N, std::size_t Alignment>
    template<typename T>
    T& InplaceTrivialAny<N, Alignment>::Get() {
        return *reinterpret_cast<T*>(&buffer_);
    }

    template<std::size_t N, std::size_t Alignment>
    template<typename T>
    const T& InplaceTrivialAny<N, Alignment>::Get() const {
        return *reinterpret_cast<const T*>(&buffer_);
    }

    template<std::size_t N, std::size_t Alignment>
    template<typename T>
    void InplaceTrivialAny<N, Alignment>::Copy(T&& other) {
        std::memcpy(reinterpret_cast<char*>(&buffer_), reinterpret_cast<const char*>(&other), sizeof(T));
    }

    template<std::size_t N, std::size_t Alignment>
    constexpr std::size_t InplaceTrivialAny<N, Alignment>::GetCapacity() noexcept {
        return N;
    }

} // End of namespace fast_containers

#endif //FAST_CONTAINERS_INPLACE_ANY_H
