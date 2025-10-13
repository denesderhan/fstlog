//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstddef>
#include <type_traits>

#include <config_parser.hpp>
#include <detail/safe_reinterpret_cast.hpp>
#include <detail/unaligned_span.hpp>
#include <fstlog/detail/aggregate_type.hpp>
#include <fstlog/detail/char_type.hpp>
#include <fstlog/detail/error_code.hpp>
#include <fstlog/detail/fstlog_assert.hpp>
#include <fstlog/detail/hash_type.hpp>
#include <fstlog/detail/log_element_type.hpp>
#include <fstlog/detail/log_policy.hpp>
#include <fstlog/detail/log_type_metadata.hpp>
#include <fstlog/detail/small_string.hpp>
#include <fstlog/detail/str_hash_fnv.hpp>
#include <fstlog/detail/types.hpp>
#include <fstlog/detail/ut_cast.hpp>

namespace fstlog {
    template<typename L>
    class arg_parser_mixin : public L {
    public:
        using memory_resource_type = typename L::memory_resource_type;
        using format_type = typename L::format_type;

        explicit arg_parser_mixin(memory_resource_type* resource) noexcept(
            std::is_nothrow_constructible_v<L, memory_resource_type*>) 
            : L(resource) {}

        arg_parser_mixin(const arg_parser_mixin& other) noexcept(
            noexcept(other.get_memory_resource())
            && std::is_nothrow_constructible_v<
                arg_parser_mixin,
                const arg_parser_mixin&,
                memory_resource_type*>)
            : arg_parser_mixin(other, other.get_memory_resource()) {}
        arg_parser_mixin(
            const arg_parser_mixin& other, 
            memory_resource_type* resource) noexcept(
                std::is_nothrow_constructible_v<
                    L,
                    const arg_parser_mixin&,
                    memory_resource_type*>)
            : L(other, resource) {}

        arg_parser_mixin(arg_parser_mixin&& other) = delete;
        arg_parser_mixin& operator=(const arg_parser_mixin& rhs) = delete;
        arg_parser_mixin& operator=(arg_parser_mixin&& rhs) = delete;
        
        ~arg_parser_mixin() = default;
        
        void process_char(log_element_ut meta, format_type format) noexcept {
            if (meta == ut_cast(char_type::Char)
                || meta == ut_cast(char_type::Char8))
            {
                // char8_t is reinterpreted to char
                char to_encode{'!'};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (meta == ut_cast(char_type::Char16)) {
                char16_t to_encode{u'!'};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (meta == ut_cast(char_type::Char32)) {
                char32_t to_encode{U'!'};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }

        void process_string(log_element_ut meta, format_type format) noexcept {
            if (meta == ut_cast(char_type::Char)
                || meta == ut_cast(char_type::Char8))
            {
                // char, char8_t is reinterpreted to unsigned char
                byte_span_const to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (meta == ut_cast(char_type::Char16)) {
                unaligned_span<const char16_t> to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (meta == ut_cast(char_type::Char32)) {
                unaligned_span<const char32_t> to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }

        void process_small_string(
            log_element_ut meta, 
            format_type format) noexcept 
        {
            // char, is reinterpreted to unsigned char
            if (meta == 0) {
                small_string<16> to_encode;
                this->get_data(to_encode);
                if(!this->has_error()) 
                    this->encode(byte_span_const { 
                    safe_reinterpret_cast<const unsigned char*>(to_encode.data()), 
                    to_encode.size() }, format);
            }
            else if (meta == 1) {
                small_string<32> to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(byte_span_const{
                    safe_reinterpret_cast<const unsigned char*>(to_encode.data()),
                    to_encode.size() }, format);
            }
            else if (meta == 2) {
                small_string<64> to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(byte_span_const{
                    safe_reinterpret_cast<const unsigned char*>(to_encode.data()),
                    to_encode.size() }, format);
            }
            else if (meta == 4) {
                small_string<128> to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(byte_span_const{
                     safe_reinterpret_cast<const unsigned char*>(to_encode.data()),
                     to_encode.size() }, format);
            }
            else if (meta == 8) {
                small_string<256> to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(byte_span_const{
                    safe_reinterpret_cast<const unsigned char*>(to_encode.data()),
                    to_encode.size() }, format);
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }
        
        void process_integral(log_element_ut meta, format_type format) noexcept {
            bool const int_signed = meta & 0b1000;
            std::size_t const int_size = 
                std::size_t{ 1 } << (meta & 0b111);
            if (int_size == sizeof(int)) {
                if (int_signed) {
                    int to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
                else {
                    unsigned int to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
            }
            else if (int_size == sizeof(long)) {
                if (int_signed) {
                    long to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
                else {
                    unsigned long to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
            }
            else if (int_size == sizeof(long long)) {
                if (int_signed) {
                    long long to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
                else {
                    unsigned long long to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
            }
            else if (int_size == sizeof(short)) {
                if (int_signed) {
                    short to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
                else {
                    unsigned short to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
            }
            else if (int_size == sizeof(unsigned char)) {
                if (int_signed) {
                    signed char to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
                else {
                    unsigned char to_encode{0};
                    this->get_data(to_encode);
                    if (!this->has_error())
                        this->encode(to_encode, format);
                }
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }

        void process_float(log_element_ut meta, format_type format) noexcept {
            std::size_t const float_size = 
                std::size_t{ 1 } << (meta & 0b111);
            if (float_size == sizeof(float)) {
                float to_encode{0.0f};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (float_size == sizeof(double)) {
                double to_encode{0.0};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (float_size == sizeof(long double)) {
                long double to_encode{ (long double)(0.0) };
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }

        void process_hash(log_element_ut meta, format_type format) noexcept {
            if (meta == ut_cast(hash_type::Fnv)) {
                str_hash_fnv to_encode;
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }

        void process_simple_type(
            log_element_type type, 
            log_element_ut meta, 
            format_type format) noexcept
        {
            if (type == log_element_type::Smallstring) {
                process_small_string(meta, format);
            }
            else if (type == log_element_type::Integral) {
                process_integral(meta, format);
            }
            else if (type == log_element_type::String) {
                process_string(meta, format);
            }
            else if (type == log_element_type::Float) {
                process_float(meta, format);
            }
            else if (type == log_element_type::Pointer) {
                void* to_encode{nullptr};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else if (type == log_element_type::Hash) {
                process_hash(meta, format);
            }
            else if (type == log_element_type::Char) {
                process_char(meta, format);
            }
            else if (type == log_element_type::Bool) {
                bool to_encode{false};
                this->get_data(to_encode);
                if (!this->has_error())
                    this->encode(to_encode, format);
            }
            else {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
            }
        }

        void skip_type(byte_span_const& type_signature) noexcept {
            const int end_ind = static_cast<int>(type_signature.size_bytes());
            int str_ind = 0;
            int loop_counter = 1;
            while (loop_counter-- != 0) {
                if (str_ind >= end_ind) {
                    this->set_error(__FILE__, __LINE__, 
                        error_code::input_bad);
                    return;
                }
                const unsigned char data_type = 
                    *(type_signature.data_bytes() + str_ind);
                const log_element_type current_type = 
                    static_cast<log_element_type>(data_type & log_element_type_bitmask);
                const unsigned char meta = 
                    data_type & log_type_metadata_bitmask;
                str_ind++;
                if (current_type == log_element_type::Aggregate) {
                    if (meta == ut_cast(aggregate_type::List))
                        loop_counter++;
                    else if (meta == ut_cast(aggregate_type::Tuple)) {
                        if (str_ind >= end_ind) {
                            this->set_error(__FILE__, __LINE__, 
                                error_code::input_bad);
                            return;
                        }
                        const unsigned char tuple_size = 
                            *(type_signature.data_bytes() + str_ind);
                        str_ind++;
                        loop_counter += tuple_size;
                    }
                    else {
                        this->set_error(__FILE__, __LINE__, 
                            error_code::input_bad);
                        return;
                    }
                }
            }
            type_signature = byte_span_const{
                type_signature.data_bytes() + str_ind,
                static_cast<std::size_t>(end_ind - str_ind)
            };
        }

        void process_list(
            byte_span_const& type_signature, 
            format_type format, 
            int tree_depth) noexcept
        {
            FSTLOG_ASSERT(!type_signature.empty());
            FSTLOG_ASSERT(*type_signature.data_bytes() ==
                (ut_cast(log_element_type::Aggregate) 
                    | ut_cast(aggregate_type::List)));
            msg_counter list_size{0};
            this->get_data(list_size);
            if (this->has_error()) return;
            this->encode_aggregate_start(aggregate_type::List, list_size);
            byte_span_const tmp_type_signature{
                type_signature.data_bytes() + 1,
                type_signature.size_bytes() - 1 
            };
            for (msg_counter i = 0; i < list_size; i++) {
                type_signature = tmp_type_signature;
                process_element(type_signature, format, tree_depth);
                if (this->has_error()) return;
                if (i + 1 < list_size) {
                    this->encode_aggregate_element_separator();
                }
            }
            this->encode_aggregate_stop();
            if (list_size == 0) {
                skip_type(type_signature);
            }
        }

        void process_tuple(
            byte_span_const& type_signature, 
            format_type format, 
            int tree_depth) noexcept
        {
            if (type_signature.size_bytes() < 2) {
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
                return;
            }
            FSTLOG_ASSERT(    *type_signature.data_bytes() ==
                (ut_cast(log_element_type::Aggregate) 
                    | ut_cast(aggregate_type::Tuple)));
            const unsigned char tuple_size = *(type_signature.data_bytes() + 1);
            type_signature = byte_span_const{
                type_signature.data_bytes() + 2,
                type_signature.size_bytes() - 2 
            };
            this->encode_aggregate_start(aggregate_type::Tuple, tuple_size);
            for (int i = 0; i < tuple_size; i++) {
                process_element(type_signature, format, tree_depth);
                if (this->has_error()) return;
                if (i + 1 < tuple_size) {
                    this->encode_aggregate_element_separator();
                }
            }
            this->encode_aggregate_stop();
        }

        void process_element(
            byte_span_const& type_signature, 
            format_type format, 
            int tree_depth = 0) noexcept
        {
            if (!type_signature.empty() && !this->has_error()) {
                auto data_type = *type_signature.data_bytes();
                log_element_type const current_type =
                    static_cast<log_element_type>(data_type & log_element_type_bitmask);
                const unsigned char meta = data_type & log_type_metadata_bitmask;

                if (current_type != log_element_type::Aggregate) {
                    process_simple_type(current_type, meta, format);
                    type_signature = byte_span_const{
                        type_signature.data_bytes() + 1,
                        type_signature.size_bytes() - 1
                    };
                }
                else {
                    tree_depth++;
                    if (tree_depth > config::max_parser_tree_depth) {
                        this->set_error(__FILE__, __LINE__, 
                            error_code::recur_lim);
                        //cant use skip_type() alone, should make skip_data too!!
                    }
                    else if (meta == ut_cast(aggregate_type::List)) {
                        process_list(type_signature, format, tree_depth);
                    }
                    else if (meta == ut_cast(aggregate_type::Tuple)) {
                        process_tuple(type_signature, format, tree_depth);
                    }
                    else {
                        this->set_error(__FILE__, __LINE__, 
                            error_code::input_bad);
                    }
                }
            }
            else if (!this->has_error())
                this->set_error(__FILE__, __LINE__, 
                    error_code::input_bad);
        }
    };
}
