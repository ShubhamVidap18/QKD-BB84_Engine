#include "QBER.hpp"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{

bool approximatelyEqual(
    double lhs,
    double rhs,
    double tolerance = 1.0e-12)
{
    return std::fabs(lhs - rhs) <= tolerance;
}

void testZeroErrorKey()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U, 1U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 0U, 1U, 1U, 0U
    };

    const QBERResult result =
        qber.estimate(aliceKey, bobKey);

    assert(result.comparedBits == 6U);
    assert(result.errorBits == 0U);
    assert(approximatelyEqual(result.qber, 0.0));
    assert(result.errorFree());
}

void testKnownErrorRate()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 1U, 1U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 1U, 1U, 0U
    };

    const QBERResult result =
        qber.estimate(aliceKey, bobKey);

    /*
     * Alice: 0 1 0 1 1
     * Bob:   0 1 1 1 0
     *
     * Errors:
     *   index 2
     *   index 4
     *
     * QBER = 2 / 5 = 0.4
     */
    assert(result.comparedBits == 5U);
    assert(result.errorBits == 2U);
    assert(approximatelyEqual(result.qber, 0.4));
    assert(!result.errorFree());
}

void testAllBitsDifferent()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey{
        0U, 0U, 0U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        1U, 1U, 1U, 1U
    };

    const QBERResult result =
        qber.estimate(aliceKey, bobKey);

    assert(result.comparedBits == 4U);
    assert(result.errorBits == 4U);
    assert(approximatelyEqual(result.qber, 1.0));
    assert(!result.errorFree());
}

void testEmptyKeyRejected()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey;
    const std::vector<std::uint8_t> bobKey;

    bool exceptionThrown = false;

    try
    {
        static_cast<void>(
            qber.estimate(aliceKey, bobKey)
        );
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

void testDifferentKeySizesRejected()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U
    };

    bool exceptionThrown = false;

    try
    {
        static_cast<void>(
            qber.estimate(aliceKey, bobKey)
        );
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

void testInvalidAliceBitRejected()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 2U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 0U, 0U
    };

    bool exceptionThrown = false;

    try
    {
        static_cast<void>(
            qber.estimate(aliceKey, bobKey)
        );
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

void testInvalidBobBitRejected()
{
    QBER qber;

    const std::vector<std::uint8_t> aliceKey{
        0U, 1U, 0U, 0U
    };

    const std::vector<std::uint8_t> bobKey{
        0U, 1U, 3U, 0U
    };

    bool exceptionThrown = false;

    try
    {
        static_cast<void>(
            qber.estimate(aliceKey, bobKey)
        );
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

} // namespace

int main()
{
    std::cout
        << "============================================================\n"
        << "                    QBER TEST SUITE\n"
        << "============================================================\n\n";

    testZeroErrorKey();
    std::cout << "[PASS] Zero-error sifted key\n";

    testKnownErrorRate();
    std::cout << "[PASS] Known QBER calculation\n";

    testAllBitsDifferent();
    std::cout << "[PASS] Maximum QBER calculation\n";

    testEmptyKeyRejected();
    std::cout << "[PASS] Empty sifted key rejected\n";

    testDifferentKeySizesRejected();
    std::cout << "[PASS] Mismatched key sizes rejected\n";

    testInvalidAliceBitRejected();
    std::cout << "[PASS] Invalid Alice bit rejected\n";

    testInvalidBobBitRejected();
    std::cout << "[PASS] Invalid Bob bit rejected\n";

    std::cout
        << "\n============================================================\n"
        << "                 ALL QBER TESTS PASSED\n"
        << "============================================================\n";

    return 0;
}