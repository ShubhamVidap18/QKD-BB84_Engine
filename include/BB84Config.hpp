#pragma once

#include "BB84config.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bb84 {

struct BB84Config {
    std::size_t raw_bit_count = 1024;
    double qber_threshold = 0.11;
    std::size_t minimum_key_bytes = 16;
    std::uint32_t key_expiry_seconds = 3600;

    // Production deployments should provide a cryptographically secure
    // RNG and an authenticated classical channel through dedicated modules.
    bool require_authenticated_classical_channel = true;
};

} // namespace bb84
