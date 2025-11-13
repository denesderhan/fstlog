//Copyright © 2022, Dénes Derhán.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#include <fstlog/output/output_stream_mt.hpp>

#include <detail/make_allocated.hpp>
#include <detail/mixin/memory_resource_mixin.hpp>
#include <detail/mixin/concurrent_use_mixin.hpp>
#include <detail/mixin/mutex_external_mixin.hpp>
#include <detail/mixin/reference_counter_mixin.hpp>
#include <output/impl/out_stream_mixin.hpp>
#include <output/output_interface_mixin.hpp>
#include <output/output_locked_mixin.hpp>

namespace fstlog {
   using output_stream_mt_impl_type = 
        output_interface_mixin<
        output_locked_mixin<
        out_stream_mixin<
        concurrent_use_mixin<
        reference_counter_mixin<
        mutex_external_mixin<
        memory_resource_mixin>>>>>>;

   error_code output_stream_mt(
       output& out,
       std::shared_ptr<std::ostream> stream,
       std::shared_ptr<std::mutex> mutex,
       memory_resource* resource) noexcept
   {
       out = output{ make_allocated<output_stream_mt_impl_type>(resource) };
       if (out.pimpl() == nullptr) return error_code::alloc_fail;
       auto* const pimpl = static_cast<output_stream_mt_impl_type*>(out.pimpl());
       pimpl->set_memory_resource(resource);
       auto error = pimpl->set_mutex(std::move(mutex));
       if (error == error_code::none) {
           error = pimpl->set_stream(std::move(stream));
       }
       if (error != error_code::none) {
           out = output{};
       }
       return error;
   }
}
