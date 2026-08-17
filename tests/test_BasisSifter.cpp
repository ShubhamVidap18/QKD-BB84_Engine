#include "BasisSifter.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

void testMatchingBasesAreRetained()
{
    const std::vector<std::uint8_t> aliceBits = {
        0U, 1U, 0U, 1U, 1U
    };

    const std::vector<BB84Basis> aliceBases = {
        BB84Basis::Z,
        BB84Basis::X,
        BB84Basis::Z,
        BB84Basis::X,
        BB84Basis::Z
    };

    const std::vector<std::uint8_t> bobBits = {
        0U, 0U, 0U, 1U, 1U
    };

    const std::vector<BB84Basis> bobBases = {
        BB84Basis::Z,
        BB84Basis::Z,
        BB84Basis::Z,
        BB84Basis::X,
        BB84Basis::X
    };

    const BasisSiftResult result =
        BasisSifter::sift(
            aliceBits,
            aliceBases,
            bobBits,
            bobBases
        );

    /*
     * Matching positions:
     *
     * 0 -> Z == Z
     * 2 -> Z == Z
     * 3 -> X == X
     *
     * Positions 1 and 4 are discarded.
     */

    assert(result.matchingIndices.size() == 3U);

    assert(result.matchingIndices[0] == 0U);
    assert(result.matchingIndices[1] == 2U);
    assert(result.matchingIndices[2] == 3U);

    assert(result.aliceKey.size() == 3U);
    assert(result.bobKey.size() == 3U);

    assert(result.aliceKey[0] == 0U);
    assert(result.aliceKey[1] == 0U);
    assert(result.aliceKey[2] == 1U);

    assert(result.bobKey[0] == 0U);
    assert(result.bobKey[1] == 0U);
    assert(result.bobKey[2] == 1U);
}


void testDifferentBasesAreDiscarded()
{
    const std::vector<std::uint8_t> aliceBits = {
        0U, 1U, 0U
    };

    const std::vector<BB84Basis> aliceBases = {
        BB84Basis::Z,
        BB84Basis::Z,
        BB84Basis::Z
    };

    const std::vector<std::uint8_t> bobBits = {
        1U, 0U, 1U
    };

    const std::vector<BB84Basis> bobBases = {
        BB84Basis::X,
        BB84Basis::X,
        BB84Basis::X
    };

    const BasisSiftResult result =
        BasisSifter::sift(
            aliceBits,
            aliceBases,
            bobBits,
            bobBases
        );

    // No basis matches, therefore nothing is retained.
    assert(result.matchingIndices.empty());
    assert(result.aliceKey.empty());
    assert(result.bobKey.empty());
}


void testMismatchedInputSizesAreRejected()
{
    const std::vector<std::uint8_t> aliceBits = {
        0U, 1U
    };

    const std::vector<BB84Basis> aliceBases = {
        BB84Basis::Z,
        BB84Basis::X
    };

    const std::vector<std::uint8_t> bobBits = {
        0U
    };

    const std::vector<BB84Basis> bobBases = {
        BB84Basis::Z,
        BB84Basis::X
    };

    bool exceptionThrown = false;

    try {
        static_cast<void>(
            BasisSifter::sift(
                aliceBits,
                aliceBases,
                bobBits,
                bobBases
            )
        );
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}


void testInvalidBitIsRejected()
{
    const std::vector<std::uint8_t> aliceBits = {
        0U, 2U
    };

    const std::vector<BB84Basis> aliceBases = {
        BB84Basis::Z,
        BB84Basis::X
    };

    const std::vector<std::uint8_t> bobBits = {
        0U, 1U
    };

    const std::vector<BB84Basis> bobBases = {
        BB84Basis::Z,
        BB84Basis::X
    };

    bool exceptionThrown = false;

    try {
        static_cast<void>(
            BasisSifter::sift(
                aliceBits,
                aliceBases,
                bobBits,
                bobBases
            )
        );
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}


int main()
{
    testMatchingBasesAreRetained();
    testDifferentBasesAreDiscarded();
    testMismatchedInputSizesAreRejected();
    testInvalidBitIsRejected();

    std::cout << "BasisSifter tests passed.\n";

    return 0;
}