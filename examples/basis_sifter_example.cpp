#include "Alice.hpp"
#include "Bob.hpp"
#include "BasisSifter.hpp"
#include "PhotonGenerator.hpp"
#include "BasisSelector.hpp"
#include "OpenSSLRandomGenerator.hpp"

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

void printBitVector(
    const std::string& label,
    const std::vector<std::uint8_t>& values)
{
    std::cout << label << ": ";

    if (values.empty())
    {
        std::cout << "<empty>\n";
        return;
    }

    for (const std::uint8_t value : values)
    {
        std::cout << static_cast<unsigned int>(value);
    }

    std::cout << '\n';
}

void printBasisVector(
    const std::string& label,
    const std::vector<BB84Basis>& bases)
{
    std::cout << label << ": ";

    if (bases.empty())
    {
        std::cout << "<empty>\n";
        return;
    }

    for (const BB84Basis basis : bases)
    {
        std::cout << basisToString(basis) << ' ';
    }

    std::cout << '\n';
}

} // namespace

int main()
{
    OpenSSLRandomGenerator randomGenerator;

    BasisSelector aliceBasisSelector(randomGenerator);
    //BasisSelector bobBasisSelector(randomGenerator);

    PhotonGenerator photonGenerator(randomGenerator);

    Alice alice(
         photonGenerator,
         aliceBasisSelector
    );

    Bob bob(randomGenerator);

    std::cout << '\n';
    std::cout
        << "============================================================\n";
    std::cout
        << "                  BB84 BASIS SIFTING\n";
    std::cout
        << "============================================================\n\n";

    /*
     * =========================================================
     * PURPOSE
     * =========================================================
     *
     * This example demonstrates the BB84 basis-sifting stage.
     *
     * Alice has already:
     *
     *   1. Generated random classical bits.
     *   2. Selected preparation bases.
     *   3. Prepared/encoded the corresponding BB84 states.
     *
     * Bob has already:
     *
     *   1. Received the quantum states.
     *   2. Selected measurement bases.
     *   3. Measured the received states.
     *
     * BasisSifter performs the classical basis reconciliation:
     *
     *   Alice basis == Bob basis
     *       -> RETAIN
     *
     *   Alice basis != Bob basis
     *       -> DISCARD
     *
     * This component does NOT perform:
     *
     *   - quantum state generation
     *   - basis selection
     *   - measurement
     *   - QBER estimation
     *   - error correction
     *   - privacy amplification
     *   - key storage
     *   - KMS operations
     *
     * The purpose of this example is to validate that the
     * BasisSifter correctly retains only matching-basis events.
     * =========================================================
     */

    constexpr std::size_t stateCount = 8U;

    /*
     * =========================================================
     * ALICE INPUT
     * =========================================================
     *
     * These represent Alice's classical bit values and the
     * preparation basis used for each transmission event.
     */

    const std::vector<AliceTransmission> transmissions =
    alice.prepareTransmission(stateCount);

    std::vector<std::uint8_t> aliceBits;
    std::vector<BB84Basis> aliceBases;

    aliceBits.reserve(stateCount);
    aliceBases.reserve(stateCount);

    for (const AliceTransmission& transmission : transmissions)
    {
       aliceBits.push_back(transmission.rawBit);
       aliceBases.push_back(transmission.basis);
    }
    /*
     * =========================================================
     * BOB INPUT
     * =========================================================
     *
     * Bob has measured the received quantum states.
     *
     * For this example we model an ideal transmission:
     *
     * whenever Alice and Bob use the same basis, Bob obtains
     * the same bit as Alice.
     *
     * When their bases differ, Bob's measurement result is
     * discarded during sifting.
     */

    std::vector<BB84State> quantumStates;
    quantumStates.reserve(stateCount);

    for (const AliceTransmission& transmission : transmissions)
    {
         quantumStates.push_back(transmission.state);
    }

    const MeasurementBatch bobResults =
    bob.measure(quantumStates);

    std::vector<std::uint8_t> bobMeasurements;
    std::vector<BB84Basis> bobBases;

    bobMeasurements.reserve(stateCount);
    bobBases.reserve(stateCount);

    for (const MeasurementResult& measurement : bobResults.results)
    {
        bobMeasurements.push_back(measurement.bit);
        bobBases.push_back(measurement.basis);
    }    
    /*
     * =========================================================
     * INPUT VALIDATION
     * =========================================================
     */

    if (aliceBits.size() != stateCount ||
        aliceBases.size() != stateCount ||
        bobMeasurements.size() != stateCount ||
        bobBases.size() != stateCount)
    {
        std::cerr
            << "ERROR: Input vector sizes do not match "
            << "the configured state count.\n";

        return 1;
    }

    /*
     * =========================================================
     * DISPLAY INPUT
     * =========================================================
     */

    std::cout
        << "Transmission states : "
        << stateCount
        << "\n\n";

    printBitVector(
        "Alice raw bits",
        aliceBits
    );

    printBasisVector(
        "Alice bases",
        aliceBases
    );

    printBitVector(
        "Bob measurements",
        bobMeasurements
    );

    printBasisVector(
        "Bob bases",
        bobBases
    );

    std::cout << '\n';

    /*
     * =========================================================
     * PERFORM BASIS SIFTING
     * =========================================================
     *
     * BasisSifter performs deterministic classical processing.
     *
     * No randomness is generated here.
     */

    std::cout
        << "Performing BB84 basis reconciliation...\n\n";

    const BasisSiftResult result =
        BasisSifter::sift(
            aliceBits,
            aliceBases,
            bobMeasurements,
            bobBases
        );

    /*
     * =========================================================
     * DISPLAY SIFTING DECISIONS
     * =========================================================
     */

    std::cout
        << "------------------------------------------------------------\n";
    std::cout
        << "                     SIFTING RESULTS\n";
    std::cout
        << "------------------------------------------------------------\n\n";

    std::cout
        << std::left
        << std::setw(8)  << "Index"
        << std::setw(10) << "A-Bit"
        << std::setw(10) << "A-Basis"
        << std::setw(10) << "B-Bit"
        << std::setw(10) << "B-Basis"
        << std::setw(14) << "Decision"
        << '\n';

    std::cout
        << "------------------------------------------------------------\n";

    for (std::size_t index = 0U;
         index < stateCount;
         ++index)
    {
        const bool basisMatch =
            aliceBases[index] == bobBases[index];

        const char* decision =
            basisMatch
                ? "RETAIN"
                : "DISCARD";

        std::cout
            << std::left
            << std::setw(8)
            << index

            << std::setw(10)
            << static_cast<unsigned int>(
                   aliceBits[index])

            << std::setw(10)
            << basisToString(
                   aliceBases[index])

            << std::setw(10)
            << static_cast<unsigned int>(
                   bobMeasurements[index])

            << std::setw(10)
            << basisToString(
                   bobBases[index])

            << std::setw(14)
            << decision

            << '\n';
    }

    std::cout
        << "------------------------------------------------------------\n\n";

    /*
     * =========================================================
     * SIFTING SUMMARY
     * =========================================================
     */

    std::cout << "Sifting summary:\n";

    std::cout
        << "  Total transmission events : "
        << stateCount
        << '\n';

    std::cout
        << "  Retained events           : "
        << result.retainedCount
        << '\n';

    std::cout
        << "  Discarded events          : "
        << result.discardedCount
        << '\n';

    std::cout
        << "  Sifted key length         : "
        << result.aliceKey.size()
        << " bits\n\n";

    /*
     * =========================================================
     * DISPLAY SIFTED KEY MATERIAL
     * =========================================================
     *
     * These are still sifted/raw key materials.
     *
     * They have NOT yet undergone:
     *
     *   - QBER estimation
     *   - error correction
     *   - privacy amplification
     *
     * Therefore this is not yet the final secure QKD key.
     */

    printBitVector(
        "Alice sifted key",
        result.aliceKey
    );

    printBitVector(
        "Bob sifted key",
        result.bobKey
    );

    /*
     * =========================================================
     * VERIFY RETAINED INDICES
     * =========================================================
     */

    std::cout << "\nMatching transmission indices: ";

    if (result.matchingIndices.empty())
    {
        std::cout << "<none>";
    }
    else
    {
        for (const std::size_t index :
             result.matchingIndices)
        {
            std::cout << index << ' ';
        }
    }

    std::cout << "\n";

    /*
     * =========================================================
     * CONSISTENCY VALIDATION
     * =========================================================
     *
     * This validation only checks whether Alice and Bob's
     * retained bits are identical for this controlled example.
     *
     * It is NOT QBER estimation.
     *
     * A production QKD implementation will perform proper
     * parameter estimation and QBER calculation in a separate
     * component.
     */

    const bool keysMatch =
        result.aliceKey == result.bobKey;

    std::cout << '\n';

    if (keysMatch)
    {
        std::cout
            << "Sifted key consistency : PASSED\n";
    }
    else
    {
        std::cout
            << "Sifted key consistency : DIFFERENCE DETECTED\n";
    }

    /*
     * =========================================================
     * RESULT VALIDATION
     * =========================================================
     *
     * For this example:
     *
     *   Total events = 8
     *   Matching bases = 5
     *   Discarded = 3
     *
     * The implementation must therefore return:
     *
     *   retainedCount  = 5
     *   discardedCount = 3
     *   sifted length  = 5
     */

    const bool resultStructureValid =
    (result.retainedCount + result.discardedCount == stateCount) &&
    (result.matchingIndices.size() == result.retainedCount) &&
    (result.aliceKey.size() == result.retainedCount) &&
    (result.bobKey.size() == result.retainedCount);

    const bool validationPassed =
        resultStructureValid && keysMatch;
        std::cout << '\n';

    if (validationPassed)
    {
        std::cout
            << "BasisSifter validation : PASSED\n";
    }
    else
    {
        std::cout
            << "BasisSifter validation : FAILED\n";
    }

    /*
     * =========================================================
     * FINAL STATUS
     * =========================================================
     */

    std::cout << '\n';
    std::cout
        << "============================================================\n";

    if (validationPassed)
    {
        std::cout
            << "BB84 basis sifting completed successfully.\n";
    }
    else
    {
        std::cout
            << "BB84 basis sifting validation failed.\n";
    }

    std::cout
        << "============================================================\n";

    return validationPassed ? 0 : 1;
}