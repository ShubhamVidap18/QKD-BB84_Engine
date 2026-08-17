#include "PhotonGenerator.hpp"

#include <limits>
#include <stdexcept>

PhotonGenerator::PhotonGenerator(
    IRandomGenerator& randomGenerator)
    : randomGenerator_(randomGenerator)
{
}

std::uint8_t PhotonGenerator::generateBit()
{
    std::uint8_t randomByte{0};

    randomGenerator_.generateBytes(
        std::span<std::uint8_t>(&randomByte, 1)
    );

    /*
     * A CSPRNG byte is uniformly distributed.
     *
     * Taking the least significant bit therefore gives
     * a uniformly distributed binary value.
     */
    return static_cast<std::uint8_t>(
        randomByte & 0x01U
    );
}

std::vector<std::uint8_t>
PhotonGenerator::generateBits(std::size_t count)
{
    if (count == 0U) {
        return {};
    }

    constexpr std::size_t bitsPerByte = 8U;

    const std::size_t byteCount =
        (count + bitsPerByte - 1U) / bitsPerByte;

    std::vector<std::uint8_t> randomBytes(byteCount);

    randomGenerator_.generateBytes(randomBytes);

    std::vector<std::uint8_t> bits;
    bits.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t byteIndex = i / bitsPerByte;
        const std::size_t bitIndex = i % bitsPerByte;

        const std::uint8_t bit =
            static_cast<std::uint8_t>(
                (randomBytes[byteIndex] >> bitIndex) & 0x01U
            );

        bits.push_back(bit);
    }

    return bits;
}

BB84State PhotonGenerator::generateQuantumState(
    std::uint8_t bit,
    BB84Basis basis)
{
    if (bit > 1U) {
        throw std::invalid_argument(
            "BB84 bit must be 0 or 1"
        );
    }

    switch (basis) {
    case BB84Basis::Z:
        return (bit == 0U)
            ? BB84State::Zero
            : BB84State::One;

    case BB84Basis::X:
        return (bit == 0U)
            ? BB84State::Plus
            : BB84State::Minus;

    default:
        throw std::invalid_argument(
            "Invalid BB84 basis"
        );
    }
}