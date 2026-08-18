#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief Result of QBER estimation for a sifted BB84 key.
 *
 * QBER (Quantum Bit Error Rate) represents the fraction of
 * sifted key bits for which Alice and Bob have different
 * classical bit values.
 *
 * QBER = mismatched bits / total sifted bits
 *
 * Example:
 *
 *     Alice: 0 1 0 1 1
 *     Bob:   0 1 1 1 0
 *
 *     mismatches = 2
 *     total      = 5
 *
 *     QBER = 2 / 5 = 0.40
 */
struct QBERResult {

    /**
     * @brief Number of sifted bits compared.
     */
    std::size_t comparedBits{0U};

    /**
     * @brief Number of positions where Alice and Bob differ.
     */
    std::size_t errorBits{0U};

    /**
     * @brief Quantum Bit Error Rate.
     *
     * Range:
     *
     *     0.0 <= qber <= 1.0
     *
     * A value of 0.0 means no observed errors.
     * A value of 1.0 means every compared bit differs.
     */
    double qber{0.0};

    /**
     * @brief Return true if no errors were detected.
     */
    [[nodiscard]]
    bool errorFree() const noexcept
    {
        return errorBits == 0U;
    }
};

/**
 * @brief Performs QBER estimation on sifted BB84 keys.
 *
 * QBER estimation is a classical post-processing operation.
 *
 * The estimator:
 *
 *  - operates only on sifted keys
 *  - compares Alice's and Bob's retained bits
 *  - counts mismatched bits
 *  - calculates the QBER
 *  - does not modify either input key
 *  - does not perform error correction
 *  - does not perform privacy amplification
 *  - does not generate new randomness
 *
 * The estimator must never receive Alice's raw key material
 * when used outside the BB84 engine's internal processing
 * pipeline.
 */
class QBER {
public:

    QBER() = default;

    QBER(const QBER&) = delete;
    QBER& operator=(const QBER&) = delete;

    QBER(QBER&&) = delete;
    QBER& operator=(QBER&&) = delete;

    ~QBER() = default;

    /**
     * @brief Estimate QBER between Alice's and Bob's sifted keys.
     *
     * @param aliceKey Alice's sifted key.
     * @param bobKey Bob's sifted key.
     *
     * @return QBER estimation result.
     *
     * @throws std::invalid_argument if:
     *         - key sizes differ
     *         - a key contains a value other than 0 or 1
     */
    [[nodiscard]]
    QBERResult estimate(
        const std::vector<std::uint8_t>& aliceKey,
        const std::vector<std::uint8_t>& bobKey
    ) const;
};
