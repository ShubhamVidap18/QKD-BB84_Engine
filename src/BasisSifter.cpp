#include "BasisSifter.hpp"

#include <stdexcept>

BasisSiftResult BasisSifter::sift(
    const std::vector<std::uint8_t>& aliceBits,
    const std::vector<BB84Basis>& aliceBases,
    const std::vector<std::uint8_t>& bobBits,
    const std::vector<BB84Basis>& bobBases)
{
    /*
     * All four vectors represent the same sequence of
     * BB84 transmission events.
     *
     * Therefore their sizes must be identical.
     */

    if (aliceBits.size() != aliceBases.size())
    {
        throw std::invalid_argument(
            "Alice bits and Alice bases must have identical sizes"
        );
    }

    if (bobBits.size() != bobBases.size())
    {
        throw std::invalid_argument(
            "Bob bits and Bob bases must have identical sizes"
        );
    }

    if (aliceBits.size() != bobBits.size())
    {
        throw std::invalid_argument(
            "Alice and Bob input sequences must have identical sizes"
        );
    }

    /*
     * Validate Alice's classical bits.
     *
     * BB84 classical bits are binary:
     *
     *      0 or 1
     */

    for (const std::uint8_t bit : aliceBits)
    {
        if (bit > 1U)
        {
            throw std::invalid_argument(
                "Alice bit sequence contains an invalid bit"
            );
        }
    }

    /*
     * Validate Bob's measurement results.
     */

    for (const std::uint8_t bit : bobBits)
    {
        if (bit > 1U)
        {
            throw std::invalid_argument(
                "Bob bit sequence contains an invalid bit"
            );
        }
    }

    BasisSiftResult result;

    const std::size_t transmissionCount = aliceBits.size();

    /*
     * Reserve maximum possible output capacity.
     *
     * In the best case every transmission uses matching
     * bases.
     */

    result.matchingIndices.reserve(transmissionCount);
    result.aliceKey.reserve(transmissionCount);
    result.bobKey.reserve(transmissionCount);

    /*
     * Perform basis reconciliation.
     *
     * Matching basis:
     *
     *     Alice basis == Bob basis
     *                 |
     *                 +--> retain
     *
     * Different basis:
     *
     *     Alice basis != Bob basis
     *                 |
     *                 +--> discard
     */

    for (std::size_t index = 0U;
         index < transmissionCount;
         ++index)
    {
        if (aliceBases[index] == bobBases[index])
        {
            /*
             * Matching basis.
             */

            result.matchingIndices.push_back(index);

            result.aliceKey.push_back(
                aliceBits[index]
            );

            result.bobKey.push_back(
                bobBits[index]
            );

            ++result.retainedCount;
        }
        else
        {
            /*
             * Different basis.
             */

            ++result.discardedCount;
        }
    }

    return result;
}