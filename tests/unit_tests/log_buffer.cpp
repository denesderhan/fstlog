//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <config_buffer.hpp>
#include <detail/nearest_pow2.hpp>
#include <detail/log_buffer_impl.hpp>
#include <detail/log_buffer_unread_data.hpp>

TEST_CASE("log_buffer") {
	SECTION("size_corner_case") {
		auto extent = GENERATE(table<uint32_t, uint32_t>({
			std::tuple<uint32_t, uint32_t>{0, fstlog::config::default_ringbuffer_size},
			std::tuple<uint32_t, uint32_t>{1, fstlog::config::min_ringbuffer_size},
			std::tuple<uint32_t, uint32_t>{UINT32_MAX, fstlog::config::max_ringbuffer_size}
			}));

		uint32_t log_buff_size_set = std::get<0>(extent);
		uint32_t log_buff_size_expected = std::get<1>(extent);

		fstlog::log_buffer_impl test_log_buffer(log_buff_size_set);

		CHECK((uintptr_t(test_log_buffer.write_ptr()) % fstlog::constants::cache_ls_nosharing) == 0);
		CHECK((uintptr_t(test_log_buffer.write_ptr()) % fstlog::constants::internal_msg_alignment) == 0);
		CHECK((uintptr_t(test_log_buffer.write_ptr()) % fstlog::config::buffer_alignment) == 0);
		CHECK(test_log_buffer.size() == log_buff_size_expected);
	};

	SECTION("wrap_around_update_producer_state") {
		fstlog::log_buffer_impl test_log_buffer(1024);
		using index_type = decltype(test_log_buffer.write_pos_)::value_type;

		index_type custom_pos{ 0 };
		custom_pos -= static_cast<index_type>(fstlog::constants::internal_msg_alignment);

		test_log_buffer.write_pos_ = custom_pos;
		test_log_buffer.read_pos_ = custom_pos;
		test_log_buffer.writeable_size_ = fstlog::constants::internal_msg_alignment;

		const auto wptr_0{ test_log_buffer.write_ptr() };

		CHECK(wptr_0 == test_log_buffer.begin_
			+ test_log_buffer.size() 
			- fstlog::constants::internal_msg_alignment);
		CHECK(!test_log_buffer.half_full());

		fstlog::log_buffer_unread_data unr;
		test_log_buffer.get_unread(unr);
		CHECK((unr.size1 == 0 && unr.size2 == 0));

		test_log_buffer.update_producer_state(2 * fstlog::constants::internal_msg_alignment);
		CHECK((test_log_buffer.write_pos_ == 0
			&& test_log_buffer.write_ptr() == test_log_buffer.begin_));
		CHECK(!test_log_buffer.half_full());
		test_log_buffer.advance_write_pos(2 * fstlog::constants::internal_msg_alignment);
		CHECK(!test_log_buffer.half_full());
		test_log_buffer.get_unread(unr);
		CHECK((unr.pos1 == wptr_0 
			&& unr.size1 == fstlog::constants::internal_msg_alignment));
		CHECK((unr.pos2 == test_log_buffer.begin_
			&& unr.size2 == 2 * fstlog::constants::internal_msg_alignment));
		test_log_buffer.advance_read_pos(unr.size1 + unr.size2);
		CHECK(test_log_buffer.read_pos_ == 2 * fstlog::constants::internal_msg_alignment);
		CHECK(!test_log_buffer.half_full());
	};

	SECTION("wrap_around_advance_write_pos") {
		fstlog::log_buffer_impl test_log_buffer(1024);
		using index_type = decltype(test_log_buffer.write_pos_)::value_type;

		index_type custom_pos{ 0 };
		custom_pos -= static_cast<index_type>(fstlog::constants::internal_msg_alignment);

		test_log_buffer.write_pos_ = custom_pos;
		test_log_buffer.read_pos_ = custom_pos;
		test_log_buffer.writeable_size_ = fstlog::constants::internal_msg_alignment;

		const auto wptr_0{ test_log_buffer.write_ptr() };

		CHECK(wptr_0 == test_log_buffer.begin_
			+ test_log_buffer.size()
			- fstlog::constants::internal_msg_alignment);

		fstlog::log_buffer_unread_data unr;
		test_log_buffer.get_unread(unr);
		CHECK((unr.size1 == 0 && unr.size2 == 0));
		CHECK(!test_log_buffer.half_full());
		test_log_buffer.advance_write_pos(fstlog::constants::internal_msg_alignment);
		CHECK(!test_log_buffer.half_full());
		CHECK((test_log_buffer.write_pos_ == 0
			&& test_log_buffer.write_ptr() == test_log_buffer.begin_));
		test_log_buffer.get_unread(unr);
		CHECK((unr.pos1 == wptr_0
			&& unr.size1 == fstlog::constants::internal_msg_alignment));
		CHECK(unr.size2 == 0);
		test_log_buffer.advance_read_pos(unr.size1 + unr.size2);
		CHECK(!test_log_buffer.half_full());
		CHECK(test_log_buffer.read_pos_ == 0);
	};
}

TEST_CASE("log_buffer_random", "[.][random]") {
	SECTION("init_size") {
		uint32_t log_buff_size_set = GENERATE(take(100000, random(uint32_t(fstlog::config::min_ringbuffer_size), uint32_t(fstlog::config::max_ringbuffer_size))));
		uint32_t log_buff_size_expected = fstlog::detail::nearest_pow2(log_buff_size_set);
		if (log_buff_size_set == 0) log_buff_size_expected = fstlog::config::default_ringbuffer_size;

		fstlog::log_buffer_impl test_log_buffer(log_buff_size_set);
		CAPTURE(log_buff_size_set);
		CHECK((uintptr_t(test_log_buffer.write_ptr()) % fstlog::constants::cache_ls_nosharing) == 0);
		CHECK((uintptr_t(test_log_buffer.write_ptr()) % fstlog::constants::internal_msg_alignment) == 0);
		CHECK((uintptr_t(test_log_buffer.write_ptr()) % fstlog::config::buffer_alignment) == 0);
		CHECK(test_log_buffer.size() == log_buff_size_expected);
	}
}
