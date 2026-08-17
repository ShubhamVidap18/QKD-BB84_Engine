#pragma once

#include "BasisSelector.hpp"
#include "PhotonGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief One prepared BB84 transmission.
 *
 * This represents Alice's internal preparation result.
 *
 * rawBit:
 *      Alice's original randomly generated classical bit.
 *
 * basis:
 *      Basis selected by Alice for encoding.
 *
 * state:
 *      BB84 quantum state corresponding to rawBit + basis.
 */
struct AliceTransmission {
    std::uint8_t rawBit;
    BB84Basis basis;
    BB84State state;
};

/**
 * @brief Performs Alice-side BB84 preparation.
 *
 * Alice is responsible for:
 *  - generating random raw bits
 *  - selecting random preparation bases
 *  - encoding bits into BB84 states
 *
 * Alice does NOT perform:
 *  - Bob measurement
 *  - sifting
 *  - QBER estimation
 *  - error correction
 *  - privacy amplification
 */
class Alice {
public:
    /**
     * @brief Construct Alice with the required dependencies.
     *
     * Both dependencies must remain valid for the lifetime
     * of the Alice object.
     */
    Alice(
        PhotonGenerator& photonGenerator,
        BasisSelector& basisSelector
    );

    ~Alice() = default;

    Alice(const Alice&) = delete;
    Alice& operator=(const Alice&) = delete;

    Alice(Alice&&) = delete;
    Alice& operator=(Alice&&) = delete;

    /**
     * @brief Generate Alice's raw random bits.
     */
    std::vector<std::uint8_t> generateRawBits(
        std::size_t count
    );

    /**
     * @brief Select Alice's independent preparation bases.
     */
    std::vector<BB84Basis> selectBases(
        std::size_t count
    );

    /**
     * @brief Encode bits and bases into BB84 states.
     *
     * @param bits Alice's raw bits.
     * @param bases Alice's selected bases.
     *
     * @return Prepared BB84 transmission sequence.
     *
     * @throws std::invalid_argument if the sizes differ.
     */
    std::vector<AliceTransmission> encode(
        const std::vector<std::uint8_t>& bits,
        const std::vector<BB84Basis>& bases
    );

    /**
     * @brief Prepare a complete BB84 transmission.
     *
     * Generates bits, selects bases, and encodes them.
     */
    std::vector<AliceTransmission> prepareTransmission(
        std::size_t count
    );

private:
    PhotonGenerator& photonGenerator_;
    BasisSelector& basisSelector_;
};