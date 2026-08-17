#pragma once

#include "IRandomGenerator.hpp"
#include "MeasurementResult.hpp"
#include "PhotonGenerator.hpp"

#include <cstddef>
#include <vector>

/**
 * Bob is the receiving party in the BB84 protocol.
 *
 * Responsibilities:
 *  1. Select random measurement bases.
 *  2. Measure received BB84 quantum states.
 *  3. Produce measurement results.
 *
 * Bob must not have access to Alice's raw bits or preparation bases.
 */
class Bob {
public:
    explicit Bob(IRandomGenerator& randomGenerator);

    Bob(const Bob&) = delete;
    Bob& operator=(const Bob&) = delete;

    Bob(Bob&&) = delete;
    Bob& operator=(Bob&&) = delete;

    ~Bob() = default;

    /**
     * Measure a sequence of received BB84 states.
     *
     * @param states Quantum states received through the quantum channel.
     *
     * @return Bob's measurement results.
     *
     * @throws std::invalid_argument if the input contains an invalid state.
     */
    [[nodiscard]]
    MeasurementBatch measure(
        const std::vector<BB84State>& states
    );

private:
    IRandomGenerator& randomGenerator_;

    [[nodiscard]]
    std::uint8_t generateRandomBit();

    [[nodiscard]]
    BB84Basis selectBasis();

    [[nodiscard]]
    std::uint8_t measureState(
        BB84State state,
        BB84Basis measurementBasis
    );};