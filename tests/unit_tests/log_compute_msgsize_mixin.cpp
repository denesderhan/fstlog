//Copyright © Dénes Derhán 2022.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <catch2/catch_all.hpp>

#include <array>
#include <vector> 
#include <cstdint>
#include <type_traits>

#include <fstlog/detail/types.hpp>
#include <detail/mixin/allocator_mixin.hpp>
#include <fstlog/detail/padded_size.hpp>
#include <fstlog/logger/detail/log/log_compute_msgsize_mixin.hpp>
#include <fstlog/logger/detail/logger_msgsize_mixin.hpp>
#include <fstlog/logger/detail/logger_writer_mixin.hpp>
#include <fstlog/logger/log_policy_lowlatency.hpp>

class mock_buffer {
public:
	void set_size(std::uint32_t size) noexcept {
		max_size = size;
	}
	std::uint32_t max_message_size() const noexcept {
		return max_size;
	}
	std::uint32_t max_size{ 0 };
};

template<class L>
class mock_tester : public L {
public:
	template<
		fstlog::level level,
		template<class T> class policy,
		fstlog::log_call_flag flags,
		typename... Args>
	constexpr void log([[maybe_unused]] Args const&... args) {
		test_size = L::msg_size();
		test_argnum = L::arg_num();
	}
	
	void init(std::uint32_t max_size) {
		buff.set_size(max_size);
	}

	mock_buffer& buffer() {
		return buff;
	}

	auto get_size() {
		return test_size;
	}
	auto get_argnum() {
		return test_argnum;
	}

	mock_buffer buff;
	typename L::msg_size_type test_size{ 0 };
	typename L::arg_num_type test_argnum{ 0 };
};


using w_type = fstlog::log_compute_msgsize_mixin<
	mock_tester<
	fstlog::logger_msgsize_mixin<
	fstlog::allocator_mixin>>>;

TEST_CASE("log_compute_msgsize_mixin") {

	SECTION("compile_time_compsize") {
		w_type comp;
		auto extent = GENERATE(table<std::uint32_t, std::uint32_t, std::uint32_t>({
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{20}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size), std::uint32_t{0}},
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{32}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size + fstlog::log_arg_size<int>()), std::uint32_t{1} }
			}));

		std::uint32_t max_size = std::get<0>(extent);
		std::uint32_t control_size = std::get<1>(extent);
		std::uint32_t control_argnum = std::get<2>(extent);
		comp.init(max_size);
		comp.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(42, std::array<int, 3>{0,1,2}, 0);
		CAPTURE(max_size, comp.get_size(), 
			control_size, comp.get_size(),
			control_argnum, comp.get_argnum());
		CHECK(comp.get_size() == control_size);
		CHECK(comp.get_argnum() == control_argnum);
	};

	SECTION("runtime_compsize") {
		w_type comp;
		std::vector<int> data{ 0,1,2 };
		CHECK(fstlog::log_arg_size(data) == 24);
		auto extent = GENERATE(table<std::uint32_t, std::uint32_t, std::uint32_t>({
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{20}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size), std::uint32_t{0}},
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{32}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size + fstlog::log_arg_size<int>()), std::uint32_t{1}},
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{40}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size + fstlog::log_arg_size<int>()), std::uint32_t{1}},
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{56}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size + fstlog::log_arg_size<int>() + 24), std::uint32_t{2}},
			std::tuple<std::uint32_t, std::uint32_t, std::uint32_t>{std::uint32_t{200}, static_cast<std::uint32_t>(fstlog::internal_msg_header::padded_data_size + 2 * fstlog::log_arg_size<int>() + 24), std::uint32_t{3}}
		}));

		std::uint32_t max_size = std::get<0>(extent);
		std::uint32_t control_size = std::get<1>(extent);
		std::uint32_t control_argnum = std::get<2>(extent);
		comp.init(max_size);
		comp.log<fstlog::level::Info, fstlog::log_policy_lowlatency, 0>(42, std::array<int, 3>{0, 1, 2}, 0);
		CAPTURE(max_size, comp.get_size(),
			control_argnum, comp.get_argnum());
		CHECK(comp.get_size() == control_size);
		CHECK(comp.get_argnum() == control_argnum);
	};
}
