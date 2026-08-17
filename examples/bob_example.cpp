#include "Bob.hpp"
#include "OpenSSLRandomGenerator.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

namespace {

const char* basisToString(BB84Basis basis)
{
    return basis == BB84Basis::Z ? "Z" : "X";
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

    default:
        return "INVALID";
    }
}

} // namespace

int main()
{
    OpenSSLRandomGenerator rng;

    Bob bob(rng);

    /*
     * These states represent states received from Alice
     * through the quantum channel.
     *
     * In the real architecture, these will NOT be manually
     * constructed. They will come from QuantumChannel.
     */
    const std::vector<BB84State> receivedStates{
        BB84State::Zero,
        BB84State::One,
        BB84State::Plus,
        BB84State::Minus,
        BB84State::Zero,
        BB84State::Plus,
        BB84State::One,
        BB84State::Minus
    };

    const MeasurementBatch measurements =
        bob.measure(receivedStates);

    std::cout << "\n";
    std::cout << "==============================================\n";
    std::cout << "             Bob BB84 Measurement\n";
    std::cout << "==============================================\n\n";

    std::cout << "Randomness:\n";
    std::cout << "  Source : OpenSSL CSPRNG\n";
    std::cout << "  Purpose: Bob measurement-basis selection\n\n";

    std::cout << "Received states: "
              << receivedStates.size()
              << "\n\n";

    std::cout << "Index | Received State | Bob Basis | Measurement\n";
    std::cout << "------------------------------------------------\n";

    for (std::size_t i = 0;
         i < measurements.results.size();
         ++i) {

        const auto& result = measurements.results[i];

        std::cout << "  "
                  << i
                  << "   |      "
                  << stateToString(receivedStates[i])
                  << "       |     "
                  << basisToString(result.basis)
                  << "     |      "
                  << static_cast<unsigned>(result.bit)
                  << '\n';
    }

    std::cout << "\n";
    std::cout << "Bob measurement completed successfully.\n";
    std::cout << "==============================================\n";

    return 0;
}