#include "Bob.hpp"

#include <cstdint>
#include <span>
#include <stdexcept>

Bob::Bob(IRandomGenerator& randomGenerator)
    : randomGenerator_(randomGenerator)
{
}

std::uint8_t Bob::generateRandomBit()
{
    std::uint8_t randomByte{0};

    randomGenerator_.generateBytes(
        std::span<std::uint8_t>(&randomByte, 1)
    );

    return static_cast<std::uint8_t>(randomByte & 0x01U);
}

BB84Basis Bob::selectBasis()
{
    const std::uint8_t randomBit = generateRandomBit();

    return (randomBit == 0U)
        ? BB84Basis::Z
        : BB84Basis::X;
}

std::uint8_t Bob::measureState(
    BB84State state,
    BB84Basis measurementBasis)
{
    switch (measurementBasis) {

    case BB84Basis::Z:

        switch (state) {

        case BB84State::Zero:
            return 0U;

        case BB84State::One:
            return 1U;

        case BB84State::Plus:
        case BB84State::Minus:
            /*
             * X-basis state measured using Z basis.
             * The result is probabilistic.
             */
            return generateRandomBit();

        default:
            throw std::invalid_argument(
                "Invalid BB84 quantum state"
            );
        }

    case BB84Basis::X:

        switch (state) {

        case BB84State::Plus:
            return 0U;

        case BB84State::Minus:
            return 1U;

        case BB84State::Zero:
        case BB84State::One:
            /*
             * Z-basis state measured using X basis.
             * The result is probabilistic.
             */
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

MeasurementBatch Bob::measure(
    const std::vector<BB84State>& states)
{
    MeasurementBatch batch;

    batch.results.reserve(states.size());

    for (const BB84State state : states) {

        const BB84Basis measurementBasis =
            selectBasis();

        const std::uint8_t measurement =
            measureState(
                state,
                measurementBasis
            );

        batch.results.push_back(
            MeasurementResult{
                measurement,
                measurementBasis
            }
        );
    }

    return batch;
}