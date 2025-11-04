//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <fstlog/detail/error_handling.hpp>
#include <fstlog/detail/memory_resource.hpp>
#include <fstlog/detail/api_def.hpp>

namespace fstlog {
    class formatter_interface;
    class formatter {
    public:
        FSTLOG_API formatter() noexcept;
        FSTLOG_API ~formatter() noexcept;
        FSTLOG_API formatter(const formatter& other) noexcept;
        FSTLOG_API formatter& operator=(const formatter& other) noexcept;
        FSTLOG_API formatter(formatter&& other) noexcept;
        FSTLOG_API formatter& operator=(formatter&& other) noexcept;
        FSTLOG_API bool operator==(const formatter& other) const noexcept;
        FSTLOG_API bool operator!=(const formatter& other) const noexcept;
        
        FSTLOG_API bool good() const noexcept;
        
        FSTLOG_TEST_API explicit formatter(formatter_interface* pimpl) noexcept;
        FSTLOG_TEST_API formatter_interface* pimpl() const noexcept;
    private:
        formatter_interface* pimpl_{ nullptr };
    };
}
