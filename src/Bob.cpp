#include "Bob.hpp"

#include <cstdint>
#include <span>
#include <stdexcept>

// Initializes Bob with the injected random number generator.
Bob::Bob(IRandomGenerator& randomGenerator)
    : randomGenerator_(randomGenerator)
{
}

// Generates a single random bit (0 or 1).
std::uint8_t Bob::generateRandomBit()
{
    std::uint8_t randomByte{0};

    // Generate one random byte and use its least significant bit.
    randomGenerator_.generateBytes(
        std::span<std::uint8_t>(&randomByte, 1)
    );

    return static_cast<std::uint8_t>(randomByte & 0x01U);
}

// Randomly selects Bob's measurement basis: Z or X.
BB84Basis Bob::selectBasis()
{
    const std::uint8_t randomBit = generateRandomBit();

    // 0 -> Z basis, 1 -> X basis.
    return (randomBit == 0U)
        ? BB84Basis::Z
        : BB84Basis::X;
}

// Measures a BB84 state using the specified measurement basis.
std::uint8_t Bob::measureState(
    BB84State state,
    BB84Basis measurementBasis)
{
    switch (measurementBasis) {

    // Measurement using the computational (Z) basis.
    case BB84Basis::Z:

        switch (state) {

        // Matching Z-basis states produce deterministic results.
        case BB84State::Zero:
            return 0U;

        case BB84State::One:
            return 1U;

        // X-basis states measured in Z produce random results.
        case BB84State::Plus:
        case BB84State::Minus:
            return generateRandomBit();

        default:
            throw std::invalid_argument(
                "Invalid BB84 quantum state"
            );
        }

    // Measurement using the diagonal (X) basis.
    case BB84Basis::X:

        switch (state) {

        // Matching X-basis states produce deterministic results.
        case BB84State::Plus:
            return 0U;

        case BB84State::Minus:
            return 1U;

        // Z-basis states measured in X produce random results.
        case BB84State::Zero:
        case BB84State::One:
            return generateRandomBit();

        default:
            throw std::invalid_argument(
                "Invalid BB84 quantum state"
            );
        }

    default:
        throw std::invalid_argument(
            "Invalid BB84 measurement basis"
        );
    }
}

// Measures all received quantum states and stores results with their bases.
MeasurementBatch Bob::measure(
    const std::vector<BB84State>& states)
{
    MeasurementBatch batch;

    // Pre-allocate storage for measurement results.
    batch.results.reserve(states.size());

    // Independently select a basis and measure each quantum state.
    for (const BB84State state : states) {

        const BB84Basis measurementBasis =
            selectBasis();

        const std::uint8_t measurement =
            measureState(
                state,
                measurementBasis
            );

        // Store Bob's measured bit and selected basis.
        batch.results.push_back(
            MeasurementResult{
                measurement,
                measurementBasis
            }
        );
    }

    return batch;
}
