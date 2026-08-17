#include "BasisSelector.hpp"
#include <cstdint>
#include <span>
#include <vector>

BasisSelector::BasisSelector(
    IRandomGenerator& randomGenerator)
    : randomGenerator_(randomGenerator)
{
}

BB84Basis BasisSelector::selectBasis()
{
    std::uint8_t randomByte{0};

    /*
     * Obtain one cryptographically secure random byte.
     *
     * The randomness is supplied by the injected
     * IRandomGenerator implementation.
     */
    randomGenerator_.generateBytes(
        std::span<std::uint8_t>(&randomByte, 1)
    );

    /*
     * Use the least significant bit to select the basis:
     *
     * 0 → Z basis
     * 1 → X basis
     *
     * Since the source is a CSPRNG, the bit is suitable
     * for security-sensitive basis selection.
     */
    const std::uint8_t basisBit =
        static_cast<std::uint8_t>(
            randomByte & 0x01U
        );

    return (basisBit == 0U)
        ? BB84Basis::Z
        : BB84Basis::X;
}

std::vector<BB84Basis>
BasisSelector::selectBases(std::size_t count)
{
    if (count == 0U) {
        return {};
    }

    /*
     * Generate enough random bytes to obtain one random
     * basis-selection bit per requested basis.
     *
     * This is more efficient than making one CSPRNG call
     * for every basis.
     */
    constexpr std::size_t bitsPerByte = 8U;

    const std::size_t byteCount =
        (count + bitsPerByte - 1U) / bitsPerByte;

    std::vector<std::uint8_t> randomBytes(byteCount);

    randomGenerator_.generateBytes(randomBytes);

    std::vector<BB84Basis> bases;
    bases.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t byteIndex =
            i / bitsPerByte;

        const std::size_t bitIndex =
            i % bitsPerByte;

        const std::uint8_t basisBit =
            static_cast<std::uint8_t>(
                (randomBytes[byteIndex] >> bitIndex) & 0x01U
            );

        bases.push_back(
            (basisBit == 0U)
                ? BB84Basis::Z
                : BB84Basis::X
        );
    }

    return bases;
}