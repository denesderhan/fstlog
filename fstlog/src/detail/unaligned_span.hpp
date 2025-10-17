//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <array>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include <detail/safe_reinterpret_cast.hpp>
#include <fstlog/detail/fstlog_assert.hpp>

namespace fstlog {
    template<typename T>
    class unaligned_span final {
    public:
        using element_type = T;
        using value_type = std::remove_cv_t<T>;
        using byte_type = std::conditional_t<
            std::is_const_v<T>,
            const unsigned char,
            unsigned char>;
        
        constexpr unaligned_span() noexcept = default;
        
        constexpr unaligned_span(byte_type* data_bytes, std::size_t byte_size) noexcept
            : data_bytes_{ data_bytes },
            byte_size_{ byte_size } 
        {
            FSTLOG_ASSERT(data_bytes != nullptr);
            FSTLOG_ASSERT(byte_size % sizeof(T) == 0 && "Invalid size for type!");
        }

        template<typename X = std::remove_const_t<T>,
            std::enable_if_t<!std::is_same_v<X, unsigned char>>* = nullptr>
        constexpr unaligned_span(T* data, std::size_t size) noexcept
            : data_bytes_{ safe_reinterpret_cast<byte_type*>(data) },
            byte_size_{ size * sizeof(T) }
        {
            FSTLOG_ASSERT(data != nullptr);
        }

        template<std::size_t N>
        constexpr unaligned_span(std::array<T, N>& arr) noexcept
            : data_bytes_{ safe_reinterpret_cast<byte_type*>(arr.data()) }
            , byte_size_{ N * sizeof(T) } {
        }

        // For const arrays, we always treat elements as const
        template<typename U, std::size_t N,
            std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>
            && std::is_const_v<T>>* = nullptr>
        constexpr unaligned_span(const std::array<U, N>& arr) noexcept
            : data_bytes_{ safe_reinterpret_cast<byte_type*>(arr.data()) }
            , byte_size_{ N * sizeof(U) } {
        }
        
        // Construction from C arrays are forbidden  
        // to prevent errors like unaligned_span("HI!") resulting in span length 4.
        template<std::size_t N>
        constexpr unaligned_span(T(&)[N]) = delete;
        
        constexpr operator unaligned_span<const T>() const noexcept {
            return unaligned_span<const T>{ data_bytes_, byte_size_ };
        }

        [[nodiscard]] constexpr byte_type* data_bytes() const noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            return data_bytes_;
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept {
            return byte_size_ / sizeof(T);
        }

        [[nodiscard]] constexpr std::size_t size_bytes() const noexcept {
            return byte_size_;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return byte_size_ == 0;
        }

        // Exclude the first count elements from the view, 
        // count is compile time const.
        template<std::size_t count = 1>
        constexpr unaligned_span<T>& drop_front() noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(count <= size());
            data_bytes_ += sizeof(T) * count;
            byte_size_ -= sizeof(T) * count;
            return *this;
        }

        // Exclude the last count elements from the view, 
        // count is compile time const.
        template<std::size_t count = 1>
        constexpr unaligned_span<T>& drop_back() noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(count <= size());
            byte_size_ -= sizeof(T) * count;
            return *this;
        }

        // Exclude the first count elements from the view.
        constexpr unaligned_span<T>& drop_front(std::size_t count) noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(count <= size());
            data_bytes_ += sizeof(T) * count;
            byte_size_ -= sizeof(T) * count;
            return *this;
        }

        // Exclude the last count elements from the view.
        constexpr unaligned_span<T>& drop_back(std::size_t count) noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(count <= size());
            byte_size_ -= sizeof(T) * count;
            return *this;
        }

        [[nodiscard]] unaligned_span<T> subspan(
            std::size_t offset, 
            std::size_t size) const noexcept
        {
            FSTLOG_ASSERT(size <= this->size() 
                && offset <= this->size() - size);
            return unaligned_span<T>(
                data_bytes_ + offset * sizeof(T),
                size * sizeof(T) );
        }

        struct deref;
        [[nodiscard]] constexpr deref operator[](std::size_t index) const noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(index < size());
            return deref(*this, index);
        }

        template<std::size_t index>
        [[nodiscard]] constexpr T get() const noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(index < size());
            if constexpr (std::is_same_v<value_type, unsigned char>
                || std::is_same_v<value_type, char>
                || std::is_same_v<value_type, std::byte>)
            {
                return *safe_reinterpret_cast<T*>(data_bytes_ + index);
            }
            else {
                value_type temp;
                std::memcpy(&temp, data_bytes_ + index * sizeof(T), sizeof(T));
                return temp;
            }
        }

        template<std::size_t index>
        constexpr void set(const T& other) const noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(index < size());
            static_assert(!std::is_const_v<T>, "Assignment to const T not allowed!");
            if constexpr (sizeof(T) == 1) {
                *(data_bytes_ + index) = *safe_reinterpret_cast<const byte_type*>(&other);
            }
            else {
                std::memcpy(data_bytes_ + index * sizeof(T), &other, sizeof(T));
            }
        }

        [[nodiscard]] constexpr T get(std::size_t index) const noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(index < size());
            if constexpr (std::is_same_v<value_type, unsigned char>
                || std::is_same_v<value_type, char>
                || std::is_same_v<value_type, std::byte>)
            {
                return *safe_reinterpret_cast<T*>(data_bytes_ + index);
            }
            else {
                value_type temp;
                std::memcpy(&temp, data_bytes_ + index * sizeof(T), sizeof(T));
                return temp;
            }
        }

        constexpr void set(std::size_t index, const T& other) const noexcept {
            FSTLOG_ASSERT(data_bytes_ != nullptr);
            FSTLOG_ASSERT(index < size());
            static_assert(!std::is_const_v<T>, "Assignment to const T not allowed!");
            if constexpr (sizeof(T) == 1) {
                *(data_bytes_ + index) = *safe_reinterpret_cast<const byte_type*>(&other);
            }
            else {
                std::memcpy(data_bytes_ + index * sizeof(T), &other, sizeof(T));
            }
        }

        constexpr bool operator==(const unaligned_span<T>& other) const noexcept {
            return byte_size_ == other.byte_size_
                && data_bytes_ == other.data_bytes_;
        }

        constexpr bool operator!=(const unaligned_span<T>& other) const noexcept {
            return !(*this == other);
        }

        // proxy used for differentiating get/set
        struct deref {
            constexpr deref(const unaligned_span& span, std::size_t index) noexcept
                : span_(span), index_(index) {}

            constexpr operator T() const noexcept {
                if constexpr (std::is_same_v<value_type, unsigned char>
                    || std::is_same_v<value_type, char>
                    || std::is_same_v<value_type, std::byte>)
                {
                    return *safe_reinterpret_cast<T*>(span_.data_bytes() + index_);
                }
                else {
                    value_type temp;
                    std::memcpy(&temp, span_.data_bytes() + index_ * sizeof(T), sizeof(T));
                    return temp;
                }
            }

            // can not return a reference (there is no T object)
            constexpr void operator=(const T& other) const noexcept {
                static_assert(!std::is_const_v<T>, "Assignment to const T not allowed!");
                if constexpr (sizeof(T) == 1) {
                    *(span_.data_bytes() + index_) = *safe_reinterpret_cast<byte_type*>(&other);
                }
                else {
                    std::memcpy(span_.data_bytes() + index_ * sizeof(T), &other, sizeof(T));
                }
            }
            
            const unaligned_span& span_;
            const std::size_t index_;
        };

    private:
        byte_type* data_bytes_{ nullptr };
        std::size_t byte_size_{ 0 };

        static_assert(!std::is_reference_v<value_type>, "T can't be a reference!");
        static_assert(std::is_trivially_copyable_v<value_type>, "T must be trivially copyable!");
        static_assert(std::is_standard_layout_v<value_type>, "T must be standard layout!");
    
        // ----------------------------------------------------------------------------------
        // Why there is no `T* data()` member
        // -------------------------------------------------------------------------
        // `unaligned_span` deliberately does **not** expose a `T* data()`.
        // Dereferencing a `T*` that points into raw, uninitialized memory is
        // undefined behaviour, even if the address happens to satisfy
        // `alignof(T)`.  The C++ object model requires that a `T` object be
        // *constructed* before any non‑trivial access; a plain byte buffer does
        // not provide that.  Accessing it through a `T*` violates strict‑aliasing
        // rules and the lifetime rules in §[basic.life], allowing the compiler
        // to perform optimisations that assume a valid `T` object exists.
        //
        // Preferred, safe access:
        //   Use *`operator[]` : it reads/writes internally via `memcpy`, 
        //      creating a temporary `T` object on each access.
        //   Copy the raw bytes using *`data_bytes()` and size_bytes()  
        //      with `memcpy` into a T object yourself.
        //
        // If the underlying buffer *is* already a valid, aligned array of
        // constructed `T` objects, you *could* cast `data_bytes()` to `T*`—but
        // this is **strongly discouraged** as it contradicts `unaligned_span`'s 
        // design for unaligned or non-constructed memory. Prefer copying via `operator[]` or `memcpy` to avoid UB.
        // --------------------------------------------------------------------------------
    };

    template<typename T>
    unaligned_span(T*, std::size_t) -> unaligned_span<T>;
    // Deduction guide for unaligned_span to propagate const.
    template<typename U, std::size_t N>
    unaligned_span(const std::array<U, N>&) -> unaligned_span<const U>;
    // Class-template argument deduction (CTAD) guides are rules that tell the compiler 
    // how to deduce template arguments for class templates from constructor arguments.
    // Without them, you'd need to explicitly specify template parameters.
    // Guides can be manually defined additional to the automatically generated ones.
    // For our unaligned_span, we need a guide to propagate const
    // from the container to the span's byte_type.
    
    // convenience types for byte_spans
    using byte_span = unaligned_span<unsigned char>;
    using byte_span_const = unaligned_span<const unsigned char>;
}
