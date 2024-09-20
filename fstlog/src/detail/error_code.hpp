//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
namespace fstlog {
	enum class error_code : int {
		none = 0,
		input_contract_violation,
		no_space_in_buffer,
		recursion_limit,
		external_code_error
	};
}
