//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/error_code.hpp>

namespace fstlog {
	class error {
	public:

		error() noexcept = default;
		error(const char* file, int line, error_code err) noexcept
			:file_{ file }, line_{ line }, ec_{ err } {}
		const char* message() const noexcept {
			return fstlog::error_message(ec_);
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
