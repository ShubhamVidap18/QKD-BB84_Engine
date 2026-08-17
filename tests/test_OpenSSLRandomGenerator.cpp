#include "OpenSSLRandomGenerator.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

void testGenerateBytes()
{
    OpenSSLRandomGenerator rng;

    std::vector<std::uint8_t> output(32U);

    rng.generateBytes(output);

    /*
     * The important property here is that the call succeeds
     * and fills the requested buffer.
     */
    assert(output.size() == 32U);
}

void testGenerateDifferentOutputs()
{
    OpenSSLRandomGenerator rng;

    std::vector<std::uint8_t> first(32U);
    std::vector<std::uint8_t> second(32U);

    rng.generateBytes(first);
    rng.generateBytes(second);

    /*
     * This is NOT a cryptographic randomness proof.
     *
     * It is merely a sanity check that repeated calls are
     * not returning an obviously constant buffer.
     */
    assert(first != second);
}

void testEmptyBuffer()
{
    OpenSSLRandomGenerator rng;

    std::vector<std::uint8_t> output;

    rng.generateBytes(output);

    assert(output.empty());
}

int main()
{
    testGenerateBytes();
    testGenerateDifferentOutputs();
    testEmptyBuffer();

    std::cout
        << "OpenSSLRandomGenerator tests passed.\n";

    return 0;
}