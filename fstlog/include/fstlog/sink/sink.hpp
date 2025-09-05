//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/api_def.hpp>

namespace fstlog {
    class sink_interface;
    class sink {
    public:
        FSTLOG_API sink() noexcept;
        FSTLOG_API ~sink() noexcept;
        FSTLOG_API sink(const sink& other) noexcept;
        FSTLOG_API sink& operator=(const sink& other) noexcept;
        FSTLOG_API sink(sink&& other) noexcept;
        FSTLOG_API sink& operator=(sink&& other) noexcept;
        FSTLOG_API bool operator==(const sink& other) const noexcept;
        FSTLOG_API bool operator!=(const sink& other) const noexcept;
        FSTLOG_API bool good() const noexcept;
        
        FSTLOG_TEST_API explicit sink(sink_interface* pimpl) noexcept;
        FSTLOG_TEST_API sink_interface* pimpl() const noexcept;
    private:
        sink_interface* pimpl_{ nullptr };
    };
}
