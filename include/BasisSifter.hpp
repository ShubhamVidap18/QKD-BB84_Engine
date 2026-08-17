#pragma once

#include "PhotonGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

/**
 * @brief Result produced by the BB84 basis-sifting stage.
 *
 * Basis sifting compares Alice's preparation basis with Bob's
 * measurement basis.
 *
 * Only transmission positions where both parties selected
 * the same BB84 basis are retained.
 *
 * This component does NOT perform:
 *
 *   - error correction
 *   - QBER estimation
 *   - privacy amplification
 *   - key storage
 *   - key management
 */
struct BasisSiftResult
{
    /**
     * @brief Indices where Alice and Bob used the same basis.
     */
    std::vector<std::size_t> matchingIndices;

    /**
     * @brief Alice's bits retained after basis reconciliation.
     */
    std::vector<std::uint8_t> aliceKey;

    /**
     * @brief Bob's bits retained after basis reconciliation.
     */
    std::vector<std::uint8_t> bobKey;

    /**
     * @brief Number of transmission positions retained.
     */
    std::size_t retainedCount{0U};

    /**
     * @brief Number of transmission positions discarded.
     */
    std::size_t discardedCount{0U};
};


/**
 * @brief Performs deterministic BB84 basis reconciliation.
 *
 * BasisSifter compares Alice's preparation basis with Bob's
 * measurement basis for every transmitted quantum state.
 *
 * If Alice and Bob selected the same basis, the corresponding
 * classical bits are retained.
 *
 * If the bases differ, the corresponding transmission is
 * discarded.
 *
 * The sifting operation itself does not require randomness.
 */
class BasisSifter
{
public:

    /**
     * @brief Perform BB84 basis sifting.
     *
     * @param aliceBits
     *        Alice's original classical bits.
     *
     * @param aliceBases
     *        Alice's preparation bases.
     *
     * @param bobBits
     *        Bob's measurement results.
     *
     * @param bobBases
     *        Bob's measurement bases.
     *
     * @return BasisSiftResult containing the retained
     *         transmission indices and corresponding
     *         Alice/Bob sifted key material.
     *
     * @throws std::invalid_argument
     *         If the input vector sizes do not match.
     *
     * @throws std::invalid_argument
     *         If any classical bit is not 0 or 1.
     */
    static BasisSiftResult sift(
        const std::vector<std::uint8_t>& aliceBits,
        const std::vector<BB84Basis>& aliceBases,
        const std::vector<std::uint8_t>& bobBits,
        const std::vector<BB84Basis>& bobBases
    );
};