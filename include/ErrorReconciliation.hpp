#pragma once

#include "IAuthenticatedClassicalChannel.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bb84
{

/*
 * =========================================================
 * RECONCILIATION RESULT
 * =========================================================
 *
 * Contains the final synchronized keys and reconciliation
 * statistics produced by the Cascade reconciliation engine.
 */
struct ReconciliationResult
{
    /*
     * Final key material.
     *
     * Alice's key must remain unchanged.
     * Bob's key is corrected during reconciliation.
     */
    std::vector<std::uint8_t> aliceKey;
    std::vector<std::uint8_t> bobKey;

    /*
     * Number of sifted key bits processed.
     */
    std::size_t comparedBits{0U};

    /*
     * Number of errors detected during reconciliation.
     */
    std::size_t detectedErrors{0U};

    /*
     * Number of Bob's bits corrected.
     */
    std::size_t correctedBits{0U};

    /*
     * Cascade processing statistics.
     */
    std::size_t cascadePasses{0U};
    std::size_t parityQueries{0U};
    std::size_t binarySearchQueries{0U};
    
    /*
     * Authenticated classical communication statistics.
     */
    std::size_t authenticatedMessages{0U};

    /*
     * Information disclosed through authenticated
     * classical communication.
     */
    std::size_t disclosedBits{0U};

    /*
     * Indicates whether all classical messages used by
     * reconciliation were successfully authenticated.
     */
    bool authenticationPassed{false};

    /*
     * Indicates whether Alice and Bob finished with
     * identical reconciled keys.
     */
    bool success{false};
};


/*
 * =========================================================
 * ERROR RECONCILIATION
 * =========================================================
 *
 * Performs authenticated Cascade-style reconciliation on
 * Alice's and Bob's sifted BB84 keys.
 *
 * The authenticated classical channel is used exclusively
 * for reconciliation information such as parity data.
 *
 * Alice's key is never modified.
 * Bob's key is corrected when errors are located.
 */
class ErrorReconciliation
{
public:

    /*
     * @param aliceKey
     *        Alice's sifted key.
     *
     * @param bobKey
     *        Bob's sifted key.
     *
     * @param channel
     *        Authenticated classical communication channel.
     *
     * @param initialBlockSize
     *        Initial Cascade block size.
     *
     * @param cascadePasses
     *        Number of Cascade passes.
     *
     * @return ReconciliationResult containing the final
     *         synchronization status and leakage statistics.
     *
     * @throws std::invalid_argument
     *         for invalid keys or invalid parameters.
     */
    [[nodiscard]]
    static ReconciliationResult reconcile(
        const std::vector<std::uint8_t>& aliceKey,
        const std::vector<std::uint8_t>& bobKey,
        IAuthenticatedClassicalChannel& channel,
        std::size_t initialBlockSize = 4U,
        std::size_t cascadePasses = 4U
    );

private:

    static void validateKey(
        const std::vector<std::uint8_t>& key
    );

    static void validateKeys(
        const std::vector<std::uint8_t>& aliceKey,
        const std::vector<std::uint8_t>& bobKey
    );

    [[nodiscard]]
    static std::uint8_t calculateParity(
        const std::vector<std::uint8_t>& key,
        std::size_t begin,
        std::size_t end
    );

    [[nodiscard]]
    static std::vector<std::size_t> generatePermutation(
        std::size_t keySize,
        std::size_t pass
    );

    [[nodiscard]]
    static std::size_t locateError(
        const std::vector<std::uint8_t>& aliceKey,
        std::vector<std::uint8_t>& bobKey,
        std::size_t begin,
        std::size_t end,
        IAuthenticatedClassicalChannel& channel,
        ReconciliationResult& result
    );
};

} // namespace bb84