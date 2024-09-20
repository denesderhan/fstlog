//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <detail/error_code.hpp>

namespace fstlog {
	class error {
	public:

		error() noexcept = default;
		error(const char* file, int line, error_code err) noexcept
			:file_{ file }, line_{ line }, ec_{ err } {}
		const char* message() const noexcept {
			switch (ec_) {
			case error_code::none: 
				return "No error!";
			case error_code::input_contract_violation: 
				return "Input data was out of contract (or corrupted)!";
			case error_code::no_space_in_buffer:
				return "Not enough space in output buffer!";
			case error_code::recursion_limit:
				return "Recursion limit reached!";
			case error_code::external_code_error:
				return "Error in external code!";
			default:
				return "Unknown error!";
			}
		}
		
		const char* file() const noexcept {
			if (file_ != nullptr) return file_;
			else return "?";
		}
		int line() const noexcept {
			return line_;
		}
		error_code code() const noexcept {
			return ec_;
		}
	private:
		const char* file_{ nullptr };
		int line_{ -1 };
		error_code ec_{ error_code::none};
	};
}
