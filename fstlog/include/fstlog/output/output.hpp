//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/api_def.hpp>

namespace fstlog {
	class output_interface;
	class output {
	public:
		FSTLOG_API output() noexcept;
		FSTLOG_API ~output() noexcept;
		FSTLOG_API output(const output& other) noexcept;
		FSTLOG_API output& operator=(const output& other) noexcept;
		FSTLOG_API output(output&& other) noexcept;
		FSTLOG_API output& operator=(output&& other) noexcept;
		FSTLOG_API bool good() const noexcept;
		
		explicit output(output_interface* pimpl)  noexcept;
		output_interface* pimpl() const noexcept;
	private:
		output_interface* pimpl_{ nullptr };
	};
}
