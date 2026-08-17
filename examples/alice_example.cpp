#include "Alice.hpp"
#include "BasisSelector.hpp"
#include "OpenSSLRandomGenerator.hpp"
#include "PhotonGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace {

const char* basisToString(BB84Basis basis)
{
    switch (basis) {
    case BB84Basis::Z:
        return "Z";

    case BB84Basis::X:
        return "X";
    }

    return "INVALID";
}

const char* stateToString(BB84State state)
{
    switch (state) {
    case BB84State::Zero:
        return "|0>";

    case BB84State::One:
        return "|1>";

    case BB84State::Plus:
        return "|+>";

    case BB84State::Minus:
        return "|->";
    }

    return "INVALID";
}

bool verifyEncoding(
    std::uint8_t bit,
    BB84Basis basis,
    BB84State state)
{
    if (bit > 1U) {
        return false;
    }

    if (basis == BB84Basis::Z) {
        return state ==
            (bit == 0U
                ? BB84State::Zero
                : BB84State::One);
    }

    if (basis == BB84Basis::X) {
        return state ==
            (bit == 0U
                ? BB84State::Plus
                : BB84State::Minus);
    }

    return false;
}

} // namespace

int main()
{
    constexpr std::size_t transmissionCount = 16U;

    /*
     * Production randomness source.
     *
     * The example deliberately uses OpenSSLRandomGenerator
     * rather than a deterministic test generator.
     */
    OpenSSLRandomGenerator randomGenerator;

    PhotonGenerator photonGenerator(
        randomGenerator
    );

    BasisSelector basisSelector(
        randomGenerator
    );

    Alice alice(
        photonGenerator,
        basisSelector
    );

    /*
     * Alice performs:
     *
     *   1. Generate random raw bits
     *   2. Generate random bases
     *   3. Encode bit + basis into BB84 states
     */
    const auto transmission =
        alice.prepareTransmission(
            transmissionCount
        );

    std::cout
        << "\n==============================================\n"
        << "           Alice BB84 Preparation\n"
        << "==============================================\n\n";

    std::cout
        << "Number of states: "
        << transmission.size()
        << "\n\n";

    /*
     * Diagnostic display.
     *
     * WARNING:
     * This is intentionally restricted to this example.
     * Production code must NOT log raw key material.
     */
    std::cout
        << "Index | Bit | Basis | Quantum State | Valid\n"
        << "---------------------------------------------\n";

    bool encodingCorrect = true;

    for (std::size_t i = 0U;
         i < transmission.size();
         ++i) {

        const auto& item = transmission[i];

        const bool valid =
            verifyEncoding(
                item.rawBit,
                item.basis,
                item.state
            );

        if (!valid) {
            encodingCorrect = false;
        }

        std::cout
            << "  "
            << i
            << "    |  "
            << static_cast<unsigned int>(
                   item.rawBit
               )
            << "  |   "
            << basisToString(item.basis)
            << "   |     "
            << stateToString(item.state)
            << "       | "
            << (valid ? "YES" : "NO")
            << '\n';
    }

    std::cout
        << "\n---------------------------------------------\n";

    if (encodingCorrect) {
        std::cout
            << "Alice encoding verification: PASSED\n";
    } else {
        std::cout
            << "Alice encoding verification: FAILED\n";
    }

    std::cout
        << "Randomness source: OpenSSL CSPRNG\n"
        << "==============================================\n\n";

    return encodingCorrect ? 0 : 1;
}