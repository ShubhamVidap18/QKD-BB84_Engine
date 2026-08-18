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
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace
{

const char* basisToString(BB84Basis basis) noexcept
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

const char* stateToString(BB84State state) noexcept
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

    if (bits.empty())
    {
        std::cout << "<empty>\n";
        return;
    }

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

bool validateTransmissionSizes(
    const std::vector<AliceTransmission>& transmissions,
    const std::vector<std::uint8_t>& aliceBits,
    const std::vector<BB84Basis>& aliceBases,
    const std::vector<BB84State>& preparedStates,
    const std::vector<std::uint8_t>& bobBits,
    const std::vector<BB84Basis>& bobBases,
    std::size_t expectedCount)
{
    return
        transmissions.size() == expectedCount &&
        aliceBits.size() == expectedCount &&
        aliceBases.size() == expectedCount &&
        preparedStates.size() == expectedCount &&
        bobBits.size() == expectedCount &&
        bobBases.size() == expectedCount;
}

} // namespace

int main()
{
    try
    {
        constexpr std::size_t stateCount = 8U;

        /*
         * =========================================================
         * PROJECT COMPONENTS
         * =========================================================
         *
         * OpenSSLRandomGenerator
         *      |
         *      +--> PhotonGenerator
         *      |
         *      +--> BasisSelector
         *      |
         *      +--> Bob measurement randomness
         *
         * All random operations therefore use the project's
         * CSPRNG-backed IRandomGenerator abstraction.
         */

        OpenSSLRandomGenerator randomGenerator;

        PhotonGenerator photonGenerator(
            randomGenerator
        );

        BasisSelector aliceBasisSelector(
            randomGenerator
        );

        Alice alice(
            photonGenerator,
            aliceBasisSelector
        );

        Bob bob(
            randomGenerator
        );

        QuantumChannel quantumChannel;

        QBER qber;

        /*
         * =========================================================
         * PIPELINE HEADER
         * =========================================================
         */

        std::cout << '\n';
        std::cout
            << "============================================================\n";
        std::cout
            << "             BB84 ALICE -> BOB PIPELINE\n";
        std::cout
            << "============================================================\n\n";

        std::cout
            << "Transmission states : "
            << stateCount
            << "\n\n";

        /*
         * =========================================================
         * STEP 1 — CSPRNG
         * =========================================================
         */

        std::cout
            << "[1] Initializing cryptographically secure randomness...\n";

        std::cout
            << "    RNG : OpenSSLRandomGenerator\n";

        std::cout
            << "    Status : READY\n\n";

        /*
         * =========================================================
         * STEP 2 — ALICE PREPARATION
         * =========================================================
         *
         * Alice performs:
         *
         *   1. Random raw-bit generation.
         *   2. Random basis selection.
         *   3. BB84 state preparation.
         *
         * AliceTransmission contains the relationship:
         *
         *   raw bit + preparation basis -> BB84 state
         */

        std::cout
            << "[2] Alice preparing BB84 transmission...\n";

        const std::vector<AliceTransmission> transmissions =
            alice.prepareTransmission(stateCount);

        if (transmissions.size() != stateCount)
        {
            std::cerr
                << "ERROR: Alice produced an unexpected "
                   "number of transmission states.\n";

            return 1;
        }

        std::vector<std::uint8_t> aliceBits;
        std::vector<BB84Basis> aliceBases;
        std::vector<BB84State> preparedStates;

        aliceBits.reserve(transmissions.size());
        aliceBases.reserve(transmissions.size());
        preparedStates.reserve(transmissions.size());

        for (const AliceTransmission& transmission :
             transmissions)
        {
            aliceBits.push_back(transmission.rawBit);
            aliceBases.push_back(transmission.basis);
            preparedStates.push_back(transmission.state);
        }

        std::cout
            << "    Raw bits generated : "
            << aliceBits.size()
            << '\n';

        std::cout
            << "    Bases generated    : "
            << aliceBases.size()
            << '\n';

        std::cout
            << "    States prepared    : "
            << preparedStates.size()
            << "\n\n";

        /*
         * =========================================================
         * STEP 3 — DISPLAY ALICE'S PREPARATION
         * =========================================================
         */

        printBits(
            "Alice raw bits",
            aliceBits
        );

        printBases(
            "Alice preparation bases",
            aliceBases
        );

        std::cout
            << '\n';

        /*
         * =========================================================
         * STEP 4 — QUANTUM CHANNEL
         * =========================================================
         *
         * Only BB84 quantum states are transmitted.
         *
         * Alice's raw bits and preparation bases are NOT sent
         * through QuantumChannel.
         */

        std::cout
            << "[3] Transmitting BB84 states through "
               "QuantumChannel...\n";

        const std::vector<BB84State> receivedStates =
            quantumChannel.transmit(
                preparedStates
            );

        std::cout
            << "    States transmitted : "
            << quantumChannel.transmissionCount()
            << '\n';

        std::cout
            << "    States received    : "
            << receivedStates.size()
            << '\n';

        /*
         * Current QuantumChannel implementation is an ideal
         * noiseless transport model. Therefore the transmitted
         * and received state sequences must be identical.
         */

        const bool channelIntegrity =
            preparedStates == receivedStates;

        std::cout
            << "    Channel integrity  : "
            << (channelIntegrity ? "PASSED" : "FAILED")
            << "\n\n";

        if (!channelIntegrity)
        {
            std::cerr
                << "ERROR: Quantum channel integrity "
                   "validation failed.\n";

            return 1;
        }

        /*
         * =========================================================
         * STEP 5 — BOB MEASUREMENT
         * =========================================================
         *
         * Bob:
         *
         *   1. Selects a random measurement basis.
         *   2. Measures each received BB84 state.
         *
         * Bob's basis selection and measurement randomness are
         * also backed by OpenSSLRandomGenerator.
         */

        std::cout
            << "[4] Bob measuring received quantum states...\n";

        const MeasurementBatch measurementBatch =
            bob.measure(receivedStates);

        if (measurementBatch.size() != stateCount)
        {
            std::cerr
                << "ERROR: Bob produced an unexpected "
                   "number of measurements.\n";

            return 1;
        }

        std::vector<std::uint8_t> bobBits;
        std::vector<BB84Basis> bobBases;

        bobBits.reserve(measurementBatch.size());
        bobBases.reserve(measurementBatch.size());

        for (const MeasurementResult& measurement :
             measurementBatch.results)
        {
            bobBits.push_back(measurement.bit);
            bobBases.push_back(measurement.basis);
        }

        std::cout
            << "    Measurements       : "
            << measurementBatch.size()
            << '\n';

        std::cout
            << "    Measurement bases  : "
            << bobBases.size()
            << "\n\n";

        /*
         * =========================================================
         * STEP 6 — COMPLETE TRANSMISSION TABLE
         * =========================================================
         */

        std::cout
            << "------------------------------------------------------------\n";
        std::cout
            << "              ALICE -> CHANNEL -> BOB\n";
        std::cout
            << "------------------------------------------------------------\n";

        std::cout
            << std::left
            << std::setw(8)  << "Index"
            << std::setw(10) << "A-Bit"
            << std::setw(10) << "A-Basis"
            << std::setw(10) << "State"
            << std::setw(10) << "B-Basis"
            << std::setw(10) << "B-Bit"
            << '\n';

        std::cout
            << "------------------------------------------------------------\n";

        for (std::size_t index = 0U;
             index < stateCount;
             ++index)
        {
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
                << stateToString(
                       preparedStates[index])

                << std::setw(10)
                << basisToString(
                       bobBases[index])

                << std::setw(10)
                << static_cast<unsigned int>(
                       bobBits[index])

                << '\n';
        }

        std::cout
            << "------------------------------------------------------------\n\n";

        /*
         * =========================================================
         * STEP 7 — BASIS SIFTING
         * =========================================================
         *
         * BasisSifter performs deterministic classical
         * reconciliation.
         *
         * Same basis:
         *      RETAIN
         *
         * Different basis:
         *      DISCARD
         */

        std::cout
            << "[5] Performing BB84 basis sifting...\n";

        const BasisSiftResult sifted =
            BasisSifter::sift(
                aliceBits,
                aliceBases,
                bobBits,
                bobBases
            );

        /*
         * =========================================================
         * STEP 8 — SIFTING RESULTS
         * =========================================================
         */

        std::cout
            << "\n------------------------------------------------------------\n";
        std::cout
            << "                     SIFTING RESULTS\n";
        std::cout
            << "------------------------------------------------------------\n";

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
                       bobBits[index])

                << std::setw(10)
                << basisToString(
                       bobBases[index])

                << std::setw(14)
                << decision

                << '\n';
        }

        std::cout
            << "------------------------------------------------------------\n";

        /*
         * =========================================================
         * STEP 9 — SIFTING VALIDATION
         * =========================================================
         *
         * No hardcoded expected number of retained bits is used.
         *
         * The validation is based on structural invariants:
         *
         *   retained + discarded == total
         *
         *   matchingIndices.size() == retained
         *
         *   Alice sifted key size == retained
         *
         *   Bob sifted key size == retained
         */

        const bool siftingStructureValid =
            (sifted.retainedCount +
             sifted.discardedCount == stateCount) &&

            (sifted.matchingIndices.size() ==
             sifted.retainedCount) &&

            (sifted.aliceKey.size() ==
             sifted.retainedCount) &&

            (sifted.bobKey.size() ==
             sifted.retainedCount);

        std::cout
            << "\nSifting summary:\n";

        std::cout
            << "  Total transmission events : "
            << stateCount
            << '\n';

        std::cout
            << "  Retained events           : "
            << sifted.retainedCount
            << '\n';

        std::cout
            << "  Discarded events          : "
            << sifted.discardedCount
            << '\n';

        std::cout
            << "  Sifted key length         : "
            << sifted.aliceKey.size()
            << " bits\n";

        printBits(
            "  Alice sifted key",
            sifted.aliceKey
        );

        printBits(
            "  Bob sifted key",
            sifted.bobKey
        );

        std::cout
            << "  Sifting structure         : "
            << (siftingStructureValid ? "PASSED" : "FAILED")
            << "\n";

        if (!siftingStructureValid)
        {
            std::cerr
                << "\nERROR: BasisSifter returned an "
                   "invalid result structure.\n";

            return 1;
        }

        /*
         * =========================================================
         * STEP 10 — QBER ESTIMATION
         * =========================================================
         *
         * QBER is calculated only over the sifted key.
         *
         * Discarded basis-mismatch events are excluded.
         *
         * Important:
         *
         * The equality of Alice and Bob's sifted keys is NOT used
         * as the QBER calculation. QBER independently counts the
         * bit mismatches.
         */

        std::cout
            << "\n[6] Estimating QBER from sifted keys...\n";

        if (sifted.aliceKey.empty())
        {
            std::cout
                << "    No sifted bits are available.\n"
                << "    QBER cannot be estimated.\n";

            return 0;
        }

        const QBERResult qberResult =
            qber.estimate(
                sifted.aliceKey,
                sifted.bobKey
            );

        /*
         * =========================================================
         * STEP 11 — QBER RESULTS
         * =========================================================
         */

        std::cout
            << "\n------------------------------------------------------------\n";
        std::cout
            << "                      QBER RESULT\n";
        std::cout
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
            << (qberResult.errorFree()
                    ? "YES"
                    : "NO")
            << '\n';

        /*
         * =========================================================
         * STEP 12 — PIPELINE STATUS
         * =========================================================
         */

        std::cout
            << "\n============================================================\n";
        std::cout
            << "                  PIPELINE STATUS\n";
        std::cout
            << "============================================================\n\n";

        std::cout
            << "CSPRNG\n"
            << "  -> Alice raw-bit generation\n"
            << "  -> Alice basis generation\n"
            << "  -> BB84 state preparation\n"
            << "  -> QuantumChannel\n"
            << "  -> Bob basis generation\n"
            << "  -> Bob measurement\n"
            << "  -> BasisSifter\n"
            << "  -> Sifted key\n"
            << "  -> QBER estimation\n";

        /*
         * =========================================================
         * FINAL VALIDATION
         * =========================================================
         */

        const bool qberStructureValid =
            qberResult.comparedBits ==
            sifted.aliceKey.size();

        const bool pipelinePassed =
            channelIntegrity &&
            siftingStructureValid &&
            qberStructureValid;

        std::cout
            << "\n------------------------------------------------------------\n";

        std::cout
            << "Quantum channel      : "
            << (channelIntegrity ? "PASSED" : "FAILED")
            << '\n';

        std::cout
            << "Basis sifting        : "
            << (siftingStructureValid ? "PASSED" : "FAILED")
            << '\n';

        std::cout
            << "QBER structure       : "
            << (qberStructureValid ? "PASSED" : "FAILED")
            << '\n';

        std::cout
            << "Complete pipeline    : "
            << (pipelinePassed ? "PASSED" : "FAILED")
            << '\n';

        std::cout
            << "------------------------------------------------------------\n";

        if (!pipelinePassed)
        {
            std::cerr
                << "\nBB84 Alice-to-Bob pipeline validation failed.\n";

            return 1;
        }

        std::cout
            << "\n============================================================\n";
        std::cout
            << "      ALICE -> QUANTUM CHANNEL -> BOB -> SIFTING -> QBER\n";
        std::cout
            << "                  COMPLETED SUCCESSFULLY\n";
        std::cout
            << "============================================================\n";

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "\nFATAL ERROR: "
            << exception.what()
            << '\n';

        return 1;
    }
    catch (...)
    {
        std::cerr
            << "\nFATAL ERROR: Unknown exception.\n";

        return 1;
    }
}
