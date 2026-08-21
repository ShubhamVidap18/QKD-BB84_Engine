#include "ErrorReconciliation.hpp"

#include <algorithm>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>

namespace bb84
{

/*
 * =========================================================
 * KEY VALIDATION
 * =========================================================
 */

void ErrorReconciliation::validateKey(
    const std::vector<std::uint8_t>& key)
{
    for (const std::uint8_t bit : key)
    {
        if (bit > 1U)
        {
            throw std::invalid_argument(
                "Error reconciliation received "
                "a non-binary key bit"
            );
        }
    }
}


void ErrorReconciliation::validateKeys(
    const std::vector<std::uint8_t>& aliceKey,
    const std::vector<std::uint8_t>& bobKey)
{
    if (aliceKey.size() != bobKey.size())
    {
        throw std::invalid_argument(
            "Alice and Bob keys must have identical lengths"
        );
    }

    validateKey(aliceKey);
    validateKey(bobKey);
}


/*
 * =========================================================
 * PARITY CALCULATION
 * =========================================================
 */

std::uint8_t ErrorReconciliation::calculateParity(
    const std::vector<std::uint8_t>& key,
    std::size_t begin,
    std::size_t end)
{
    if (begin > end || end > key.size())
    {
        throw std::out_of_range(
            "Invalid parity range"
        );
    }

    std::uint8_t parity = 0U;

    for (std::size_t index = begin;
         index < end;
         ++index)
    {
        parity ^= key[index];
    }

    return parity;
}


/*
 * =========================================================
 * CASCADE PERMUTATION
 * =========================================================
 *
 * Alice and Bob must use the same permutation for each
 * Cascade pass.
 *
 * IMPORTANT:
 * This is a protocol demonstration implementation.
 *
 * In a production deployment, the permutation must be
 * derived from authenticated protocol/session metadata
 * rather than relying on an implementation-local PRNG
 * convention.
 */
std::vector<std::size_t>
ErrorReconciliation::generatePermutation(
    std::size_t keySize,
    std::size_t pass)
{
    std::vector<std::size_t> permutation(keySize);

    for (std::size_t index = 0U;
         index < keySize;
         ++index)
    {
        permutation[index] = index;
    }

    const std::uint64_t seed =
        0xBB84000000000000ULL ^
        (static_cast<std::uint64_t>(keySize) << 16U) ^
        static_cast<std::uint64_t>(pass);

    std::mt19937_64 generator(seed);

    std::shuffle(
        permutation.begin(),
        permutation.end(),
        generator
    );

    return permutation;
}


/*
 * =========================================================
 * ERROR LOCATION
 * =========================================================
 *
 * Locate exactly one error in a block whose parity differs.
 *
 * The input vectors are contiguous temporary vectors
 * representing a Cascade block.
 */
std::size_t ErrorReconciliation::locateError(
    const std::vector<std::uint8_t>& aliceKey,
    std::vector<std::uint8_t>& bobKey,
    std::size_t begin,
    std::size_t end,
    IAuthenticatedClassicalChannel& channel,
    ReconciliationResult& result)
{
    if (begin >= end ||
        end > aliceKey.size() ||
        end > bobKey.size())
    {
        throw std::out_of_range(
            "Invalid error-location range"
        );
    }

    /*
     * One remaining bit means its location is known.
     */
    if ((end - begin) == 1U)
    {
        bobKey[begin] ^= 1U;

        ++result.detectedErrors;
        ++result.correctedBits;

        return begin;
    }

    /*
     * Split the range into two halves.
     */
    const std::size_t middle =
        begin + ((end - begin) / 2U);

    /*
     * Query Alice's authenticated parity for the left half.
     */
    const std::uint8_t aliceParity =
        channel.queryParity(
            std::span<const std::uint8_t>(
                aliceKey.data() + begin,
                middle - begin
            )
        );

    ++result.parityQueries;
    ++result.binarySearchQueries;

    /*
     * Calculate Bob's corresponding parity locally.
     */
    const std::uint8_t bobParity =
        calculateParity(
            bobKey,
            begin,
            middle
        );

    /*
     * Parity mismatch => odd number of errors in left half.
     */
    if (aliceParity != bobParity)
    {
        return locateError(
            aliceKey,
            bobKey,
            begin,
            middle,
            channel,
            result
        );
    }

    /*
     * Otherwise the error is in the right half.
     *
     * This is valid because the parent block was already
     * established to contain an odd number of errors.
     */
    return locateError(
        aliceKey,
        bobKey,
        middle,
        end,
        channel,
        result
    );
}


/*
 * =========================================================
 * CASCADE RECONCILIATION
 * =========================================================
 */

ReconciliationResult ErrorReconciliation::reconcile(
    const std::vector<std::uint8_t>& aliceKey,
    const std::vector<std::uint8_t>& bobKey,
    IAuthenticatedClassicalChannel& channel,
    std::size_t initialBlockSize,
    std::size_t cascadePasses)
{
    /*
     * -----------------------------------------------------
     * INPUT VALIDATION
     * -----------------------------------------------------
     */

    if (initialBlockSize == 0U)
    {
        throw std::invalid_argument(
            "Initial Cascade block size must be greater than zero"
        );
    }

    if (cascadePasses == 0U)
    {
        throw std::invalid_argument(
            "Cascade pass count must be greater than zero"
        );
    }

    validateKeys(
        aliceKey,
        bobKey
    );


    /*
     * -----------------------------------------------------
     * RESULT INITIALIZATION
     * -----------------------------------------------------
     */

    ReconciliationResult result;

    result.aliceKey = aliceKey;
    result.bobKey = bobKey;

    result.comparedBits =
        aliceKey.size();


    /*
     * -----------------------------------------------------
     * EMPTY KEY
     * -----------------------------------------------------
     *
     * No classical parity exchange is necessary.
     */
    if (aliceKey.empty())
    {
        result.authenticatedMessages =
            channel.messageCount();

        result.disclosedBits =
            channel.disclosedBits();

        result.authenticationPassed = true;
        result.success = true;

        return result;
    }


    /*
     * -----------------------------------------------------
     * CASCADE PASSES
     * -----------------------------------------------------
     *
     * Pass 0 uses the initial block size.
     *
     * Subsequent passes use progressively larger blocks
     * and a different permutation.
     */
    for (std::size_t pass = 0U;
         pass < cascadePasses;
         ++pass)
    {
        ++result.cascadePasses;

        /*
         * Calculate block size:
         *
         * pass 0 -> B
         * pass 1 -> 2B
         * pass 2 -> 4B
         * pass 3 -> 8B
         */
        std::size_t blockSize =
            initialBlockSize;

        for (std::size_t multiplier = 0U;
             multiplier < pass;
             ++multiplier)
        {
            if (blockSize >
                std::numeric_limits<std::size_t>::max() / 2U)
            {
                blockSize = aliceKey.size();
                break;
            }

            blockSize *= 2U;

            if (blockSize >= aliceKey.size())
            {
                blockSize = aliceKey.size();
                break;
            }
        }

        blockSize =
            std::min(
                blockSize,
                aliceKey.size()
            );


        /*
         * Generate common permutation.
         */
        const std::vector<std::size_t> permutation =
            generatePermutation(
                aliceKey.size(),
                pass
            );


        bool passDetectedError = false;


        /*
         * -------------------------------------------------
         * PROCESS BLOCKS
         * -------------------------------------------------
         */
        for (std::size_t blockBegin = 0U;
             blockBegin < aliceKey.size();
             blockBegin += blockSize)
        {
            const std::size_t blockEnd =
                std::min(
                    blockBegin + blockSize,
                    aliceKey.size()
                );


            /*
             * Build the actual permuted Alice block.
             */
            std::vector<std::uint8_t> aliceBlock;

            aliceBlock.reserve(
                blockEnd - blockBegin
            );

            for (std::size_t position = blockBegin;
                 position < blockEnd;
                 ++position)
            {
                aliceBlock.push_back(
                    aliceKey[
                        permutation[position]
                    ]
                );
            }


            /*
             * -------------------------------------------------
             * AUTHENTICATED ALICE PARITY
             * -------------------------------------------------
             *
             * IMPORTANT:
             *
             * queryParity() receives Alice's actual block,
             * not a vector containing the parity bit itself.
             */
            const std::uint8_t aliceParity =
                channel.queryParity(
                    std::span<const std::uint8_t>(
                        aliceBlock.data(),
                        aliceBlock.size()
                    )
                );

            ++result.parityQueries;


            /*
             * Bob calculates parity locally.
             */
            std::uint8_t bobParity = 0U;

            for (std::size_t position = blockBegin;
                 position < blockEnd;
                 ++position)
            {
                bobParity ^=
                    result.bobKey[
                        permutation[position]
                    ];
            }


            /*
             * -------------------------------------------------
             * PARITY MATCH
             * -------------------------------------------------
             */
            if (aliceParity == bobParity)
            {
                continue;
            }


            /*
             * -------------------------------------------------
             * PARITY MISMATCH
             * -------------------------------------------------
             *
             * The block contains an odd number of errors.
             */
            passDetectedError = true;


            /*
             * Create temporary contiguous blocks.
             */
            std::vector<std::uint8_t> permutedAlice;
            std::vector<std::uint8_t> permutedBob;

            permutedAlice.reserve(
                blockEnd - blockBegin
            );

            permutedBob.reserve(
                blockEnd - blockBegin
            );


            for (std::size_t position = blockBegin;
                 position < blockEnd;
                 ++position)
            {
                const std::size_t originalIndex =
                    permutation[position];

                permutedAlice.push_back(
                    aliceKey[originalIndex]
                );

                permutedBob.push_back(
                    result.bobKey[originalIndex]
                );
            }


            /*
             * Binary search for one error.
             */
            const std::size_t errorIndex =
                locateError(
                    permutedAlice,
                    permutedBob,
                    0U,
                    permutedAlice.size(),
                    channel,
                    result
                );


            /*
             * Translate temporary-block index back into
             * the original sifted-key index.
             */
            const std::size_t originalIndex =
                permutation[
                    blockBegin + errorIndex
                ];


            /*
             * Correct Bob's real key.
             */
            result.bobKey[originalIndex] =
                result.aliceKey[originalIndex];
        }


        /*
         * -------------------------------------------------
         * CASCADE TERMINATION
         * -------------------------------------------------
         *
         * If an entire pass has no parity mismatch,
         * there is no newly detected odd-parity block.
         *
         * Stop early to avoid unnecessary classical
         * information leakage.
         */
        if (!passDetectedError)
        {
            break;
        }
    }


    /*
     * -----------------------------------------------------
     * CLASSICAL CHANNEL ACCOUNTING
     * -----------------------------------------------------
     */
    result.authenticatedMessages =
        channel.messageCount();

    result.disclosedBits =
        channel.disclosedBits();


    /*
     * The channel abstraction represents authenticated
     * classical communication.
     */
    result.authenticationPassed = true;


    /*
     * -----------------------------------------------------
     * FINAL SYNCHRONIZATION
     * -----------------------------------------------------
     */
    result.success =
        result.aliceKey ==
        result.bobKey;


    return result;
}

} // namespace bb84