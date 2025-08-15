//Copyright © 2023, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
namespace fstlog {
    enum class error_code : int {
        none = 0,
        input_bad,
        buff_full,
        recur_lim,
        extern_err,
        double_init,
        alloc_fail,
        thread_fail,
        obj_null,
        obj_locked,
        cpp_err,
        str_long,
        fmt_bad,
        core_null,
        buff_null,
        path_bad
    };

    inline const char* error_message(error_code code) noexcept {
        switch (code){
            case error_code::none : return "fstlog error: No error!";
            case error_code::input_bad : return "fstlog error: Input data was malformed or corrupted!";
            case error_code::buff_full : return "fstlog error: Not enough space in output buffer!";
            case error_code::recur_lim : return "fstlog error: Recursion limit reached!";
            case error_code::extern_err : return "fstlog error: Error in external code!";
            case error_code::double_init: return "fstlog error: Object is already initialized!";
            case error_code::alloc_fail: return "fstlog error: Memory allocation failed!";
            case error_code::thread_fail: return "fstlog error: Thread execution failed!";
            case error_code::obj_null: return "fstlog error: Object does not exists!";
            case error_code::obj_locked: return "fstlog error: Object is already used, access denied!";
            case error_code::cpp_err: return "fstlog error: Missing C++ language feature!";
            case error_code::str_long: return "fstlog error: String too long!";
            case error_code::fmt_bad: return "fstlog error: Invalid fmt format string!";
            case error_code::core_null: return "fstlog error: Core not set!";
            case error_code::buff_null: return "fstlog error: Buffer not set!";
            case error_code::path_bad: return "fstlog error: Path invalid or too long!";
            default : return "fstlog error: Unknown error!";
        };
    };
}
