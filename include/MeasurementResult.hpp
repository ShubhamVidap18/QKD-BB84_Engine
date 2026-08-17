#pragma once

#include "PhotonGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * Result of Bob's measurement for one BB84 quantum state.
 *
 * Bob knows:
 *  - the basis he selected
 *  - the measurement result
 *
 * Bob does NOT know Alice's original bit at this stage.
 */
struct MeasurementResult {
    std::uint8_t bit{0};
    BB84Basis basis{BB84Basis::Z};
};

/**
 * Collection of Bob's measurements.
 */
struct MeasurementBatch {
    std::vector<MeasurementResult> results;

    [[nodiscard]]
    std::size_t size() const noexcept
    {
        return results.size();
    }
};