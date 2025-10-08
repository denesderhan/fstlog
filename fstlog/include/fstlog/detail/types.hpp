//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <cstdint>

namespace fstlog {
    using log_element_ut = unsigned char;
    using type_size_counter = unsigned char;
    using level_type = unsigned char;
    using msg_counter = std::uint16_t;
    using channel_type = unsigned char;
    using log_call_flag = unsigned char;
    using stamp_type = std::int64_t;

    enum class level : level_type;
}
