#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

/**
 * @brief Abstract source of cryptographically secure random bytes.
 *
 * Production implementations must provide cryptographically secure
 * randomness. Test implementations may provide deterministic data,
 * but must never be used in production.
 */
class IRandomGenerator {
public:
    virtual ~IRandomGenerator() = default;

    /**
     * @brief Fill the supplied buffer with random bytes.
     *
     * @param output Buffer to populate.
     *
     * @throws std::runtime_error if secure randomness cannot be generated.
     */
    virtual void generateBytes(
        std::span<std::uint8_t> output
    ) = 0;
};