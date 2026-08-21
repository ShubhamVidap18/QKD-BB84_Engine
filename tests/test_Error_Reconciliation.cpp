#include "ErrorReconciliation.hpp"
#include "AuthenticatedClassicalChannel.hpp"
#include "QBER.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace bb84;


/*
 * ============================================================
 * EMPTY KEY
 * ============================================================
 */

TEST(ErrorReconciliationTest, EmptyKeysSucceed)
{
    const std::vector<std::uint8_t> aliceKey{};

    const std::vector<std::uint8_t> bobKey{};

    AuthenticatedClassicalChannel classicalChannel;

    const ReconciliationResult result =
        ErrorReconciliation::reconcile(
            aliceKey,
            bobKey,
            classicalChannel
        );

    EXPECT_TRUE(result.success);

    EXPECT_TRUE(result.authenticationPassed);

    EXPECT_EQ(result.comparedBits, 0U);
    EXPECT_EQ(result.detectedErrors, 0U);
    EXPECT_EQ(result.correctedBits, 0U);
    EXPECT_EQ(result.disclosedBits, 0U);
    EXPECT_EQ(result.authenticatedMessages, 0U);

    EXPECT_TRUE(result.aliceKey.empty());
    EXPECT_TRUE(result.bobKey.empty());
}


/*
 * ============================================================
 * IDENTICAL KEYS
 * ============================================================
 */

TEST(ErrorReconciliationTest, IdenticalKeysRequireNoCorrection)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 1U, 0U,
        1U, 0U, 1U, 0U
    };

    const std::vector<std::uint8_t> bobKey = aliceKey;

    AuthenticatedClassicalChannel classicalChannel;

    const ReconciliationResult result =
        ErrorReconciliation::reconcile(
            aliceKey,
            bobKey,
            classicalChannel
        );

    EXPECT_TRUE(result.authenticationPassed);
    EXPECT_TRUE(result.success);

    EXPECT_EQ(result.comparedBits, aliceKey.size());

    EXPECT_EQ(result.detectedErrors, 0U);
    EXPECT_EQ(result.correctedBits, 0U);

    EXPECT_EQ(result.aliceKey, aliceKey);
    EXPECT_EQ(result.bobKey, bobKey);

    EXPECT_EQ(result.authenticatedMessages, 2U);
    EXPECT_EQ(result.disclosedBits, 2U);
}


/*
 * ============================================================
 * SINGLE BIT ERROR
 * ============================================================
 *
 * The current authenticated parity implementation detects
 * an odd-parity block mismatch but does not yet perform
 * binary-search correction.
 */

TEST(ErrorReconciliationTest, SingleBitErrorIsDetectedAndCorrected)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U,
        1U, 0U, 1U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 1U, 1U,
        1U, 0U, 1U, 0U
    };

    AuthenticatedClassicalChannel classicalChannel;

    const ReconciliationResult result =
        ErrorReconciliation::reconcile(
            aliceKey,
            bobKey,
            classicalChannel
        );

    EXPECT_TRUE(result.authenticationPassed);

    /*
     * One parity mismatch identifies an erroneous block.
     */
    EXPECT_EQ(result.detectedErrors, 1U);

    /*
     * Binary search located and corrected one bit.
     */
    EXPECT_EQ(result.correctedBits, 1U);

    /*
     * Bob's corrected key must now equal Alice's key.
     */
    EXPECT_EQ(result.bobKey, aliceKey);

    EXPECT_EQ(result.aliceKey, aliceKey);

    /*
     * Successful reconciliation means both keys are synchronized.
     */
    EXPECT_TRUE(result.success);
}

TEST(ErrorReconciliationTest, MultipleErrorsRequireCascade)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U,
        1U, 0U, 1U, 0U
    };

    /*
     * Two errors:
     *
     * index 0
     * index 7
     */
    const std::vector<std::uint8_t> bobKey{
        1U, 1U, 0U, 1U,
        1U, 0U, 1U, 1U
    };

    AuthenticatedClassicalChannel classicalChannel;

    const ReconciliationResult result =
        ErrorReconciliation::reconcile(
            aliceKey,
            bobKey,
            classicalChannel
        );

    EXPECT_TRUE(result.authenticationPassed);

    /*
     * The current pass must not falsely report that zero
     * errors exist when an odd-parity block mismatch occurs.
     */
    EXPECT_GE(result.detectedErrors, 1U);

    /*
     * At least one error can be located and corrected.
     */
    EXPECT_GE(result.correctedBits, 1U);

    /*
     * Depending on block arrangement, a single pass may not
     * resolve every error.
     *
     * Therefore final synchronization is explicitly checked.
     */
    EXPECT_TRUE(
        result.bobKey == aliceKey ||
        !result.success
    );

    /*
     * Alice's key must never be modified.
     */
    EXPECT_EQ(result.aliceKey, aliceKey);
}

/*
 * ============================================================
 * DIFFERENT KEY LENGTHS
 * ============================================================
 */

TEST(ErrorReconciliationTest, DifferentKeyLengthsAreRejected)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 0U
    };

    AuthenticatedClassicalChannel classicalChannel;

    EXPECT_THROW(
        static_cast<void>(
            ErrorReconciliation::reconcile(
                aliceKey,
                bobKey,
                classicalChannel
            )
        ),
        std::invalid_argument
    );
}


/*
 * ============================================================
 * INVALID ALICE BIT
 * ============================================================
 */

TEST(ErrorReconciliationTest, NonBinaryAliceBitIsRejected)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 2U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 0U, 0U
    };

    AuthenticatedClassicalChannel classicalChannel;

    EXPECT_THROW(
        static_cast<void>(
            ErrorReconciliation::reconcile(
                aliceKey,
                bobKey,
                classicalChannel
            )
        ),
        std::invalid_argument
    );
}


/*
 * ============================================================
 * INVALID BOB BIT
 * ============================================================
 */

TEST(ErrorReconciliationTest, NonBinaryBobBitIsRejected)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 3U, 0U
    };

    AuthenticatedClassicalChannel classicalChannel;

    EXPECT_THROW(
        static_cast<void>(
            ErrorReconciliation::reconcile(
                aliceKey,
                bobKey,
                classicalChannel
            )
        ),
        std::invalid_argument
    );
}


/*
 * ============================================================
 * INVALID BLOCK SIZE
 * ============================================================
 */

TEST(ErrorReconciliationTest, ZeroBlockSizeIsRejected)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U
    };

    const std::vector<std::uint8_t> bobKey = aliceKey;

    AuthenticatedClassicalChannel classicalChannel;

    EXPECT_THROW(
        static_cast<void>(
            ErrorReconciliation::reconcile(
                aliceKey,
                bobKey,
                classicalChannel,
                0U
            )
        ),
        std::invalid_argument
    );
}


/*
 * ============================================================
 * CUSTOM BLOCK SIZE
 * ============================================================
 */

TEST(ErrorReconciliationTest, CustomBlockSizeIsSupported)
{
    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U,
        1U, 0U, 1U, 0U,
        1U, 1U
    };

    const std::vector<std::uint8_t> bobKey = aliceKey;

    AuthenticatedClassicalChannel classicalChannel;

    /*
     * 10 bits / block size 4 = 3 blocks.
     */
    const ReconciliationResult result =
        ErrorReconciliation::reconcile(
            aliceKey,
            bobKey,
            classicalChannel,
            4U
        );

    EXPECT_TRUE(result.success);

    EXPECT_TRUE(
        result.authenticationPassed
    );

    EXPECT_EQ(
        result.comparedBits,
        aliceKey.size()
    );

    EXPECT_EQ(
        result.authenticatedMessages,
        3U
    );

    EXPECT_EQ(
        result.disclosedBits,
        3U
    );
}


/*
 * ============================================================
 * QBER + AUTHENTICATED RECONCILIATION
 * ============================================================
 *
 * BB84 pipeline:
 *
 *     Sifted Keys
 *          |
 *          v
 *         QBER
 *          |
 *          v
 * Authenticated Classical Channel
 *          |
 *          v
 *   Error Reconciliation
 */

TEST(
    ErrorReconciliationIntegrationTest,
    QBERThenAuthenticatedReconciliation
)
{
    const std::vector<std::uint8_t> aliceSiftedKey{
        0U, 1U, 0U, 1U,
        1U, 0U, 1U, 0U
    };

    /*
     * Bob contains one bit error at index 2.
     */
    const std::vector<std::uint8_t> bobSiftedKey{
        0U, 1U, 1U, 1U,
        1U, 0U, 1U, 0U
    };


    /*
     * ========================================================
     * QBER
     * ========================================================
     */

    QBER qberEstimator;

    const QBERResult qber =
        qberEstimator.estimate(
            aliceSiftedKey,
            bobSiftedKey
        );

    EXPECT_EQ(
        qber.comparedBits,
        aliceSiftedKey.size()
    );

    EXPECT_EQ(
        qber.errorBits,
        1U
    );


    /*
     * ========================================================
     * AUTHENTICATED CLASSICAL RECONCILIATION
     * ========================================================
     */

    AuthenticatedClassicalChannel classicalChannel;

    const ReconciliationResult reconciliation =
        ErrorReconciliation::reconcile(
            aliceSiftedKey,
            bobSiftedKey,
            classicalChannel
        );


    /*
     * Classical communication must be authenticated.
     */
    EXPECT_TRUE(
        reconciliation.authenticationPassed
    );


    /*
     * One erroneous block is detected.
     */
    EXPECT_EQ(
        reconciliation.detectedErrors,
        1U
    );


    /*
     * Binary search locates and corrects the erroneous bit.
     */
    EXPECT_EQ(
        reconciliation.correctedBits,
        1U
    );


    /*
     * Alice's key remains unchanged.
     */
    EXPECT_EQ(
        reconciliation.aliceKey,
        aliceSiftedKey
    );


    /*
     * Bob's key must now be synchronized with Alice.
     */
    EXPECT_EQ(
        reconciliation.bobKey,
        aliceSiftedKey
    );


    /*
     * Final reconciliation must succeed.
     */
    EXPECT_TRUE(
        reconciliation.success
    );


    /*
     * Authentication statistics must be populated.
     */
    EXPECT_GT(
        reconciliation.authenticatedMessages,
        0U
    );

    EXPECT_GT(
        reconciliation.disclosedBits,
        0U
    );
}