#pragma once

#include <cstdint>
#include <memory>

namespace telecom {

// A placeholder for a single downlink Resource Block (RB).
// In real systems, an RB represents 12 subcarriers over 1 slot.
struct ResourceBlock {
    uint32_t id;
    double snr_db;
    // Add other PHY-layer fields as needed (e.g., power, modulation, etc.)
};

// RAII wrapper for a resource block allocation.
// The destructor returns the block to the pool (if applicable).
class ResourceBlockHandle {
public:
    ResourceBlockHandle() = default;
    ResourceBlockHandle(ResourceBlock rb) : rb_(std::move(rb)), valid_(true) {}

    ResourceBlockHandle(const ResourceBlockHandle&) = delete;
    ResourceBlockHandle& operator=(const ResourceBlockHandle&) = delete;

    ResourceBlockHandle(ResourceBlockHandle&& other) noexcept = default;
    ResourceBlockHandle& operator=(ResourceBlockHandle&& other) noexcept = default;

    ~ResourceBlockHandle() = default;

    bool valid() const noexcept { return valid_; }
    const ResourceBlock& get() const noexcept { return rb_; }
    ResourceBlock& get() noexcept { return rb_; }

private:
    ResourceBlock rb_{};
    bool valid_ = false;
};

} // namespace telecom
