#include "Alice.hpp"
#include "BasisSelector.hpp"
#include "BasisSifter.hpp"
#include "Bob.hpp"
#include "OpenSSLRandomGenerator.hpp"
#include "PhotonGenerator.hpp"
#include "QBER.hpp"
#include "QuantumChannel.hpp"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace
{

const char* basisToString(BB84Basis basis)
{
    switch (basis)
    {
        case BB84Basis::Z:
            return "Z";

        case BB84Basis::X:
            return "X";

        default:
            return "INVALID";
    }
}

const char* stateToString(BB84State state)
{
    switch (state)
    {
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

void printBits(
    const std::string& label,
    const std::vector<std::uint8_t>& bits)
{
    std::cout << label << ": ";

    for (const std::uint8_t bit : bits)
    {
        std::cout << static_cast<unsigned int>(bit);
    }

    std::cout << '\n';
}

void printBases(
    const std::string& label,
    const std::vector<BB84Basis>& bases)
{
    std::cout << label << ": ";

    for (const BB84Basis basis : bases)
    {
        std::cout << basisToString(basis) << ' ';
    }

    std::cout << '\n';
}

} // namespace

int main()
{
    constexpr std::size_t stateCount = 8U;

    /*
     * =========================================================
     * RANDOMNESS
     * =========================================================
     *
     * The complete example uses the project's CSPRNG-backed
     * OpenSSLRandomGenerator.
     *
     * Alice and Bob receive the same RNG abstraction, but each
     * independently consumes randomness for their own operations.
     */
    OpenSSLRandomGenerator randomGenerator;

    PhotonGenerator photonGenerator(randomGenerator);

    BasisSelector aliceBasisSelector(randomGenerator);

    Alice alice(
        photonGenerator,
        aliceBasisSelector
    );

    Bob bob(randomGenerator);

    QuantumChannel quantumChannel;

    QBER qber;

    std::cout
        << '\n'
        << "============================================================\n"
        << "                    BB84 QBER EXAMPLE\n"
        << "============================================================\n\n";

    /*
     * =========================================================
     * STEP 1 — ALICE PREPARATION
     * =========================================================
     */
    std::cout
        << "[1] Alice preparing BB84 transmission...\n";

    const std::vector<AliceTransmission> transmissions =
        alice.prepareTransmission(stateCount);

    std::vector<std::uint8_t> aliceBits;
    std::vector<BB84Basis> aliceBases;
    std::vector<BB84State> preparedStates;

    aliceBits.reserve(transmissions.size());
    aliceBases.reserve(transmissions.size());
    preparedStates.reserve(transmissions.size());

    for (const AliceTransmission& transmission : transmissions)
    {
        aliceBits.push_back(transmission.rawBit);
        aliceBases.push_back(transmission.basis);
        preparedStates.push_back(transmission.state);
    }

    std::cout
        << "    Prepared states : "
        << preparedStates.size()
        << "\n";

    /*
     * =========================================================
     * STEP 2 — QUANTUM CHANNEL
     * =========================================================
     *
     * Only BB84 quantum states enter the quantum channel.
     *
     * Alice's raw bits and preparation bases are NOT passed
     * through QuantumChannel.
     */
    std::cout
        << "\n[2] Transmitting quantum states...\n";

    const std::vector<BB84State> receivedStates =
        quantumChannel.transmit(preparedStates);

    std::cout
        << "    States transmitted : "
        << quantumChannel.transmissionCount()
        << '\n';

    std::cout
        << "    States received    : "
        << receivedStates.size()
        << '\n';

    /*
     * =========================================================
     * STEP 3 — BOB MEASUREMENT
     * =========================================================
     */
    std::cout
        << "\n[3] Bob measuring received quantum states...\n";

    const MeasurementBatch measurements =
        bob.measure(receivedStates);

    std::vector<std::uint8_t> bobBits;
    std::vector<BB84Basis> bobBases;

    bobBits.reserve(measurements.size());
    bobBases.reserve(measurements.size());

    for (const MeasurementResult& measurement :
         measurements.results)
    {
        bobBits.push_back(measurement.bit);
        bobBases.push_back(measurement.basis);
    }

    std::cout
        << "    Measurements : "
        << measurements.size()
        << '\n';

    /*
     * =========================================================
     * STEP 4 — BASIS SIFTING
     * =========================================================
     *
     * BasisSifter performs classical reconciliation.
     *
     * Only events where:
     *
     *     Alice basis == Bob basis
     *
     * are retained.
     */
    std::cout
        << "\n[4] Performing basis sifting...\n";

    const BasisSiftResult sifted =
        BasisSifter::sift(
            aliceBits,
            aliceBases,
            bobBits,
            bobBases
        );

    /*std::cout
        << "    Total events      : "
        << sifted.totalCount
        << '\n';
    */
    std::cout
        << "    Retained events   : "
        << sifted.retainedCount
        << '\n';

    std::cout
        << "    Discarded events  : "
        << sifted.discardedCount
        << '\n';

    /*
     * =========================================================
     * STEP 5 — SIFTED KEYS
     * =========================================================
     */
    std::cout << '\n';

    printBits(
        "Alice sifted key",
        sifted.aliceKey
    );

    printBits(
        "Bob sifted key",
        sifted.bobKey
    );

    /*
     * =========================================================
     * STEP 6 — QBER ESTIMATION
     * =========================================================
     *
     * QBER is calculated ONLY over the retained/sifted bits.
     *
     * Raw transmission events that were discarded because of
     * basis mismatch are not included in the QBER calculation.
     */
    std::cout
        << "\n[5] Estimating QBER...\n";

    if (sifted.aliceKey.empty())
    {
        std::cout
            << "    QBER cannot be estimated because "
            << "no bits survived sifting.\n";

        return 0;
    }

    const QBERResult qberResult =
        qber.estimate(
            sifted.aliceKey,
            sifted.bobKey
        );

    /*
     * =========================================================
     * STEP 7 — QBER RESULT
     * =========================================================
     */
    std::cout
        << "\n------------------------------------------------------------\n"
        << "                      QBER RESULT\n"
        << "------------------------------------------------------------\n";

    std::cout
        << "Compared sifted bits : "
        << qberResult.comparedBits
        << '\n';

    std::cout
        << "Error bits           : "
        << qberResult.errorBits
        << '\n';

    std::cout
        << "QBER                 : "
        << std::fixed
        << std::setprecision(4)
        << qberResult.qber
        << '\n';

    std::cout
        << "QBER percentage      : "
        << std::fixed
        << std::setprecision(2)
        << (qberResult.qber * 100.0)
        << "%\n";

    std::cout
        << "Error-free           : "
        << (qberResult.errorFree() ? "YES" : "NO")
        << '\n';

    /*
     * =========================================================
     * FINAL PIPELINE STATUS
     * =========================================================
     */
    std::cout
        << "\n============================================================\n"
        << "              BB84 QBER STAGE COMPLETED\n"
        << "============================================================\n";

    std::cout
        << "\nPipeline executed:\n\n"
        << "CSPRNG\n"
        << "  -> Alice bit generation\n"
        << "  -> Alice basis generation\n"
        << "  -> BB84 state preparation\n"
        << "  -> QuantumChannel\n"
        << "  -> Bob basis generation\n"
        << "  -> Bob measurement\n"
        << "  -> BasisSifter\n"
        << "  -> Sifted keys\n"
        << "  -> QBER estimation\n";

    std::cout
        << "\nQBER stage : PASSED\n";

    return 0;
}