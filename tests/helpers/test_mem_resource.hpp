//Copyright © Dénes Derhán 2025.
//Distributed under the AGPLv3 license (https://opensource.org/license/agpl-v3).
#pragma once
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory_resource>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <unordered_map>

class test_mem_resource : public std::pmr::memory_resource {
public:
    explicit test_mem_resource(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
        : upstream_resource_(resource)
    {
        if (!resource) throw std::invalid_argument("upstream_resource must not be null!");
    }

    test_mem_resource(const test_mem_resource&) = delete;
    test_mem_resource& operator=(const test_mem_resource&) = delete;

    ~test_mem_resource() = default;

    auto outstanding_bytes() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return outstanding_bytes_;
    }
    auto outstanding_allocations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return outstanding_allocations_;
    }
    auto peak_bytes() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return peak_allocated_bytes_;
    }
    auto peak_allocations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return peak_allocations_;
    }
    auto sum_bytes() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sum_allocated_bytes_;
    }
    auto sum_allocations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sum_allocations_;
    }

    auto matching_allocdeallocs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return matching_allocdeallocs_;
    }

    auto nonmatching_allocdeallocs() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return nonmatching_allocdeallocs_;
    }

    auto bad_allocations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return bad_allocations_;
    }

    auto bad_deallocations() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return bad_deallocations_;
    }

    struct alloc_meta {
        std::uintmax_t bytes{ 0 };
        std::uintmax_t alignment{ 1 };
    };
           
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        allocation_map_.clear();
        outstanding_allocations_ = 0;
        bad_allocations_ = 0;
        bad_deallocations_ = 0;
        matching_allocdeallocs_ = 0;
        nonmatching_allocdeallocs_ = 0;
        outstanding_bytes_ = 0;
        peak_allocated_bytes_ = 0;
        peak_allocations_ = 0;
        sum_allocated_bytes_ = 0;
        sum_allocations_ = 0;
    }

    void reset_peak() {
        std::lock_guard<std::mutex> lock(mutex_);
        peak_allocated_bytes_ = outstanding_bytes_;
        peak_allocations_ = outstanding_allocations_;
    }

    bool no_errors() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return bad_allocations_ == 0
            && bad_deallocations_ == 0
            && nonmatching_allocdeallocs_ == 0;
    }

    bool all_clear() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return allocation_map_.empty()
            && outstanding_allocations_ == 0
            && bad_allocations_ == 0
            && bad_deallocations_ == 0
            && nonmatching_allocdeallocs_ == 0;
    }

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        void* pointer{ nullptr };
        try {
            pointer = upstream_resource_->allocate(bytes, alignment);
        }
        catch(...){
            // if allocate() throws we assume no allocation occured
            std::lock_guard<std::mutex> lock(mutex_);
            bad_allocations_++;
            throw;
        }
        // if allocate() returns nullptr we assume no allocation occured
        if (pointer == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            bad_allocations_++;
            return nullptr;
        }
        
        // we assume allocation occured
        std::lock_guard<std::mutex> lock(mutex_);
        outstanding_allocations_++;
        outstanding_bytes_ += bytes;
        sum_allocated_bytes_ += bytes;
        sum_allocations_++;
        peak_allocated_bytes_ = std::max(peak_allocated_bytes_, outstanding_bytes_);
        peak_allocations_ = std::max(peak_allocations_, outstanding_allocations_);
        
        auto it = allocation_map_.find(pointer);
        // DETECTING doubly returned pointers
        if (it != allocation_map_.end()) {
            bad_allocations_++;
            return pointer;
        }
        
        // we count bad alignment
        if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
            bad_allocations_++;
        }

        allocation_map_[pointer] = alloc_meta{ bytes, alignment };
                
        return pointer;
    }

    void do_deallocate(void* pointer, std::size_t bytes, std::size_t alignment) override {
        try {
            upstream_resource_->deallocate(pointer, bytes, alignment);
        }
        catch (...) {
            // we count deallocate() fails
            std::lock_guard<std::mutex> lock(mutex_);
            bad_deallocations_++;
            throw;
        }
        // if deallocate() accepted nullptr
        if (pointer == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            bad_deallocations_++;
            return;
        }

        // we assume deallocation occured
        std::lock_guard<std::mutex> lock(mutex_);
        outstanding_allocations_--;
        outstanding_bytes_ -= bytes;
                       
        auto it = allocation_map_.find(pointer);
        // untracked allocation
        if (it == allocation_map_.end()) {
            bad_deallocations_++;
            return;
        }

        alloc_meta alloc = it->second;
        allocation_map_.erase(it);
        
        // DETECTING invalid alignment and mismatched byte/alignment
        if (alignment == 0 || (alignment & (alignment - 1)) != 0
            || alloc.bytes != bytes || alloc.alignment != alignment)
        {
            bad_deallocations_++;
        }

        // counting matching/nonmatching
        if (alloc.bytes == bytes && alloc.alignment == alignment) {
            matching_allocdeallocs_++;
        }
        else {
            nonmatching_allocdeallocs_++;
        }
    }
       
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

private:
    std::pmr::memory_resource* upstream_resource_; // The underlying memory resource
    std::unordered_map<void*, alloc_meta> allocation_map_; // Tracks alloc-dealloc pairs
    std::intmax_t outstanding_allocations_{ 0 }; // Tracks live allocations
    std::intmax_t outstanding_bytes_{ 0 }; // Tracks live allocated bytes
    std::uintmax_t matching_allocdeallocs_{ 0 }; // Tracks matching allocate-deallocate
    std::uintmax_t nonmatching_allocdeallocs_{ 0 }; // Tracks non matching (bad byte, alignment values) allocate-deallocate
    std::uintmax_t bad_deallocations_{ 0 }; // Tracks bad deallocation attempts (no pointer match)
    std::uintmax_t bad_allocations_{ 0 }; // Counts allocation attempts returning nullptr or improperly aligned pointers 
    std::intmax_t peak_allocated_bytes_{ 0 }; // maximum of concurrent bytes allocated
    std::intmax_t peak_allocations_{ 0 }; //  maximum of concurrent number of allocations
    std::intmax_t sum_allocated_bytes_{ 0 }; // all bytes allocated during lifetime of resource
    std::intmax_t sum_allocations_{ 0 }; // all number of allocations during lifetime of resource
    mutable std::mutex mutex_; // For thread safety
};

