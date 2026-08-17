#include "Alice.hpp"
#include "BasisSifter.hpp"
#include "Bob.hpp"
#include "OpenSSLRandomGenerator.hpp"
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

void printSiftedKey(
    const std::string& label,
    const std::vector<std::uint8_t>& key)
{
    std::cout << label << ": ";

    if (key.empty())
    {
        std::cout << "<empty>\n";
        return;
    }

    for (const std::uint8_t bit : key)
    {
        std::cout << static_cast<unsigned int>(bit);
    }

    std::cout << '\n';
}

} // namespace

int main()
{
    try
    {
        constexpr std::size_t stateCount = 8U;

        /*
         * =========================================================
         * RANDOMNESS
         * =========================================================
         *
         * OpenSSLRandomGenerator provides cryptographically secure
         * random bytes used by the BB84 components.
         *
         * The same RNG instance is injected into Alice and Bob.
         * Each component independently requests randomness from
         * the generator.
         */
        OpenSSLRandomGenerator randomGenerator;

        /*
         * =========================================================
         * BB84 COMPONENTS
         * =========================================================
         */

        PhotonGenerator photonGenerator(randomGenerator);

        BasisSelector aliceBasisSelector(randomGenerator);

        Alice alice(
            photonGenerator,
            aliceBasisSelector
        );

        Bob bob(randomGenerator);

        QuantumChannel quantumChannel;

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
         * STEP 1 - ALICE PREPARATION
         * =========================================================
         *
         * Alice:
         *
         *   1. Generates cryptographically secure random bits.
         *   2. Selects random BB84 preparation bases.
         *   3. Encodes each bit + basis into a BB84 state.
         */
        std::cout
            << "[1] Alice preparing BB84 transmission...\n";

        const std::vector<AliceTransmission> transmissions =
            alice.prepareTransmission(stateCount);

        if (transmissions.size() != stateCount)
        {
            std::cerr
                << "ERROR: Alice produced an unexpected number "
                << "of transmissions.\n";

            return 1;
        }

        std::cout
            << "    Prepared states : "
            << transmissions.size()
            << "\n\n";

        /*
         * Extract Alice's classical information for the later
         * sifting stage.
         *
         * In a real QKD architecture, this information would remain
         * on Alice's side and only the basis information would later
         * be exchanged through the authenticated classical channel.
         */
        std::vector<std::uint8_t> aliceBits;
        std::vector<BB84Basis> aliceBases;

        aliceBits.reserve(transmissions.size());
        aliceBases.reserve(transmissions.size());

        std::vector<BB84State> preparedStates;
        preparedStates.reserve(transmissions.size());

        for (const AliceTransmission& transmission : transmissions)
        {
            aliceBits.push_back(transmission.rawBit);
            aliceBases.push_back(transmission.basis);
            preparedStates.push_back(transmission.state);
        }

        /*
         * =========================================================
         * STEP 2 - QUANTUM CHANNEL
         * =========================================================
         *
         * Only BB84 quantum states are transmitted through the
         * QuantumChannel.
         *
         * Alice's raw bits and preparation bases are NOT passed
         * through the quantum channel.
         */
        std::cout
            << "[2] Transmitting quantum states through "
               "QuantumChannel...\n";

        const std::vector<BB84State> receivedStates =
            quantumChannel.transmit(preparedStates);

        if (receivedStates.size() != preparedStates.size())
        {
            std::cerr
                << "ERROR: Quantum channel changed the number "
                << "of transmitted states.\n";

            return 1;
        }

        std::cout
            << "    States transmitted : "
            << preparedStates.size()
            << '\n';

        std::cout
            << "    States received    : "
            << receivedStates.size()
            << '\n';

        std::cout
            << "    Channel count      : "
            << quantumChannel.transmissionCount()
            << "\n\n";

        /*
         * For the current ideal QuantumChannel implementation,
         * the received state must be identical to the transmitted
         * state.
         */
        if (preparedStates != receivedStates)
        {
            std::cerr
                << "ERROR: Quantum channel integrity check failed.\n";

            return 1;
        }

        std::cout
            << "[3] Quantum channel integrity : PASSED\n\n";

        /*
         * =========================================================
         * STEP 3 - BOB MEASUREMENT
         * =========================================================
         *
         * Bob independently:
         *
         *   1. Selects a random measurement basis.
         *   2. Measures the received BB84 state.
         *
         * Bob does not receive Alice's raw bits or preparation
         * bases as part of this operation.
         */
        std::cout
            << "[4] Bob measuring received quantum states...\n";

        const MeasurementBatch measurementBatch =
            bob.measure(receivedStates);

        if (measurementBatch.size() != stateCount)
        {
            std::cerr
                << "ERROR: Bob produced an unexpected number "
                << "of measurements.\n";

            return 1;
        }

        std::cout
            << "    Measurements : "
            << measurementBatch.size()
            << "\n\n";

        /*
         * Extract Bob's classical measurement information.
         *
         * These values are required by the subsequent sifting
         * stage.
         */
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

        /*
         * =========================================================
         * DISPLAY QUANTUM TRANSMISSION
         * =========================================================
         */

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
                << static_cast<unsigned int>(aliceBits[index])
                << std::setw(10)
                << basisToString(aliceBases[index])
                << std::setw(10)
                << stateToString(preparedStates[index])
                << std::setw(10)
                << basisToString(bobBases[index])
                << std::setw(10)
                << static_cast<unsigned int>(bobBits[index])
                << '\n';
        }

        std::cout
            << "------------------------------------------------------------\n\n";

        /*
         * =========================================================
         * STEP 4 - BASIS SIFTING
         * =========================================================
         *
         * Basis reconciliation is classical post-processing.
         *
         * Alice's and Bob's basis choices are compared:
         *
         *   Same basis
         *       -> retain the measurement
         *
         *   Different basis
         *       -> discard the measurement
         *
         * BasisSifter itself does not generate randomness and does
         * not modify the quantum states.
         */
        std::cout
            << "[5] Performing BB84 basis reconciliation...\n\n";

        const BasisSiftResult siftResult =
            BasisSifter::sift(
                aliceBits,
                aliceBases,
                bobBits,
                bobBases
            );

        /*
         * =========================================================
         * SIFTING RESULTS
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
                << static_cast<unsigned int>(aliceBits[index])
                << std::setw(10)
                << basisToString(aliceBases[index])
                << std::setw(10)
                << static_cast<unsigned int>(bobBits[index])
                << std::setw(10)
                << basisToString(bobBases[index])
                << std::setw(14)
                << decision
                << '\n';
        }

        std::cout
            << "------------------------------------------------------------\n\n";

        /*
         * =========================================================
         * SIFTING VALIDATION
         * =========================================================
         *
         * The number of retained and discarded events must account
         * for every transmission event.
         */
        const bool countConsistency =
            siftResult.retainedCount +
            siftResult.discardedCount ==
            stateCount;

        const bool indexConsistency =
            siftResult.matchingIndices.size() ==
            siftResult.retainedCount;

        const bool keySizeConsistency =
            siftResult.aliceKey.size() ==
            siftResult.retainedCount &&
            siftResult.bobKey.size() ==
            siftResult.retainedCount;

        /*
         * For the current ideal quantum channel, all retained
         * matching-basis measurements must agree.
         *
         * This is NOT QBER estimation. It is only an integrity
         * check for this ideal-channel example.
         */
        const bool siftedKeysMatch =
            siftResult.aliceKey ==
            siftResult.bobKey;

        const bool siftingPassed =
            countConsistency &&
            indexConsistency &&
            keySizeConsistency &&
            siftedKeysMatch;

        std::cout
            << "Sifting summary:\n";

        std::cout
            << "  Total transmission events : "
            << stateCount
            << '\n';

        std::cout
            << "  Retained events           : "
            << siftResult.retainedCount
            << '\n';

        std::cout
            << "  Discarded events          : "
            << siftResult.discardedCount
            << '\n';

        std::cout
            << "  Sifted key length         : "
            << siftResult.aliceKey.size()
            << " bits\n\n";

        printSiftedKey(
            "Alice sifted key",
            siftResult.aliceKey
        );

        printSiftedKey(
            "Bob sifted key",
            siftResult.bobKey
        );

        std::cout << '\n';

        std::cout
            << "Matching transmission indices: ";

        if (siftResult.matchingIndices.empty())
        {
            std::cout << "<none>";
        }
        else
        {
            for (const std::size_t index :
                 siftResult.matchingIndices)
            {
                std::cout << index << ' ';
            }
        }

        std::cout << "\n\n";

        std::cout
            << "Sifted key consistency : "
            << (siftedKeysMatch ? "PASSED" : "FAILED")
            << '\n';

        std::cout
            << "Sifting validation     : "
            << (siftingPassed ? "PASSED" : "FAILED")
            << "\n\n";

        /*
         * =========================================================
         * FINAL STATUS
         * =========================================================
         */

        if (!siftingPassed)
        {
            std::cerr
                << "ERROR: BB84 quantum transmission/sifting "
                   "validation failed.\n";

            return 1;
        }

        std::cout
            << "============================================================\n";
        std::cout
            << "       ALICE -> QUANTUM CHANNEL -> BOB -> SIFTING\n";
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