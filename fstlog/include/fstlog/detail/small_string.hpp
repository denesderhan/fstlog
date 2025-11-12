//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <string_view>
#include <type_traits>

#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/is_pow2.hpp>

namespace fstlog {
    template<std::size_t class_size>
    class alignas(is_pow2(class_size) ? class_size : 4) small_string {
    public:
        constexpr small_string() noexcept {
            data_[class_size - 1] = static_cast<char>(class_size - 1);
        };

        constexpr small_string(const char* c_str, std::size_t length) noexcept { 
            std::size_t str_length = 
                length < class_size - 1 ? length : class_size - 1;
            // can not use memset/memcpy (not constexpr!)
            for (std::size_t i = 0; i < str_length; i++) data_[i] = c_str[i];
            data_[class_size - 1] = static_cast<char>(class_size - 1 - str_length);
#ifdef FSTLOG_DEBUG
            std::size_t ind{ 0 };
            while (ind < class_size - 1 && data_[ind] != 0) ind++;
            FSTLOG_ASSERT(ind == str_length && "C string terminated with a 0 early, (actual length was shorter)");
#endif
        }

        constexpr small_string(const char* const& c_str) noexcept
            : small_string(c_str, cstr_len(c_str)) {
        }

        constexpr small_string(std::string_view str) noexcept 
            : small_string(str.data(), str.size()) {
        }

        template<std::size_t N>
        constexpr small_string(const char(&str)[N]) noexcept 
            : small_string(&str[0], N - 1) // terminating 0 is not part of the string
        {
            FSTLOG_ASSERT(str[N - 1] == 0); // if no terminating 0, the char array was not a string
        }

        constexpr small_string(const small_string&) noexcept = default;
        
        constexpr small_string& operator=(const small_string&) noexcept = default;


        [[nodiscard]] static constexpr std::size_t capacity() noexcept {
            return class_size - 1;
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept {
            return capacity() - free_size();
        }

        [[nodiscard]] constexpr std::size_t free_size() const noexcept {
            return static_cast<std::size_t>(data_[class_size - 1]);
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return free_size() == capacity();
        }

        constexpr void clear() noexcept {
            *this = small_string{};
        }

        // appends a char to the end unchecked (UB if no free space)
        constexpr void push_back(const char c) noexcept {
            FSTLOG_ASSERT(free_size() > 0);
            data_[size()] = c;
            data_[class_size - 1]--;
        }

        // removes a char from the end unchecked (UB if empty)
        constexpr void pop_back() noexcept {
            FSTLOG_ASSERT(!empty());
            data_[size() - 1] = 0;
            data_[class_size - 1]++;
        }

        [[nodiscard]] constexpr char const* data() const noexcept {
            return data_;
        }

        // writing beyond size()/end() is forbidden
        // writing \0 is forbidden
        // all unused bytes have to be 0 and 
        // the last byte of data_[] has to contain the free space at all times
        [[nodiscard]] constexpr char* data() noexcept {
            return data_;
        }

        [[nodiscard]] constexpr char const* c_str() const noexcept {
            return data_;
        }

        // writing beyond size()/end() is forbidden
        // writing \0 is forbidden
        // all unused bytes have to be 0 and 
        // the last byte of data_[] has to contain the free space at all times
        constexpr char* begin() noexcept {
            return data();
        }

        constexpr char* end() noexcept {
            return data() + size();
        }

        constexpr const char* begin() const noexcept {
            return data(); 
        }

        constexpr const char* end() const noexcept { 
            return data() + size(); 
        }

        constexpr const char* cbegin() const noexcept {
            return data();
        }

        constexpr const char* cend() const noexcept {
            return data() + size();
        }

        constexpr char const& operator[](std::size_t ind) const noexcept {
            FSTLOG_ASSERT(ind < size());
            return data_[ind];
        }

        // writing beyond size() is forbidden
        // writing \0 is forbidden
        // all unused bytes have to be 0 and 
        // the last byte of data_[] has to contain the free space at all times
        constexpr char& operator[](std::size_t ind) noexcept {
            FSTLOG_ASSERT(ind < size());
            return data_[ind];
        }

        constexpr bool operator==(
            std::string_view other) const noexcept
        {
            return std::string_view{ data(), size() } == other;
        }

        constexpr bool operator!=(
            std::string_view other) const noexcept
        {
            return !operator==(other);
        }

        constexpr bool operator<(
            std::string_view other) const noexcept
        {
            return std::string_view{ data(), size() } < other;
        }

        constexpr bool operator>(
            std::string_view other) const noexcept
        {
            return std::string_view{ data(), size() } > other;
        }

        constexpr bool operator<=(
            std::string_view other) const noexcept
        {
            return std::string_view{ data(), size() } <= other;
        }

        constexpr bool operator>=(
            std::string_view other) const noexcept
        {
            return std::string_view{ data(), size() } >= other;
        }

        // truncates if free_size() is not enough
        constexpr small_string& operator+=(
            std::string_view other) noexcept
        {
            const std::size_t other_size = other.size();
            const std::size_t free_len = free_size();
            const std::size_t copy_size = other_size < free_len ? other_size : free_len;
            std::size_t dest_pos = size();
            for (std::size_t i = 0; i < copy_size; i++) {
                data_[dest_pos++] = other[i];
            }
            data_[class_size - 1] -= static_cast<char>(copy_size);
            return *this;
        }

        // truncates if free_size() is not enough
        constexpr small_string operator+(
            std::string_view other) noexcept
        {
            small_string result{ *this };
            result += other;
            return result;
        }

        // truncates if free_size() is not enough
        constexpr small_string& operator+=(
            const char c) noexcept
        {
            char &free_len = data_[class_size - 1];
            if (free_len != 0) {
                data_[size()] = c;
                free_len--;
            }
            return *this;
        }

        // truncates if free_size() is not enough
        constexpr small_string operator+(
            const char c) noexcept
        {
            small_string result{ *this };
            result += c;
            return result;
        }

        constexpr operator std::string_view() const noexcept {
            return std::string_view{ data(), size() };
        }

        template<std::size_t N>
        constexpr operator small_string<N>() const noexcept {
            return small_string<N>{ data(), size() };
        }

        static_assert(class_size >= 4 && class_size <= 128 && class_size % 4 == 0, "Invalid size!");
#if !defined(__cpp_aligned_new) || __cpp_aligned_new < 201606
#error "Overalignment 64, 128 requires C++17’s aligned new!"
#endif
        // The last byte of data_[] is a metadata byte: 
        // stores free space count (0 to class_size-1 <= 127)
        // Fits in char even if signed, due to static_assert(class_size <= 128)
        // It acts as a terminating 0 (free space == 0) if the length is max (class_size - 1)
        char data_[class_size]{};
    
    private:
        static constexpr std::size_t cstr_len(const char* s) noexcept {
            std::size_t len = 0;
            while (s[len] != 0) ++len;
            return len;
        }
    };

    template<class T>
    struct is_small_string : std::false_type {};

    template<std::size_t N>
    struct is_small_string<small_string<N>> : std::true_type {};

    template<class T>
    inline constexpr bool is_small_string_v = is_small_string<T>::value;
}
