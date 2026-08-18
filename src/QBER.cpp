#include "QBER.hpp"

#include <stdexcept>

QBERResult QBER::estimate(
    const std::vector<std::uint8_t>& aliceKey,
    const std::vector<std::uint8_t>& bobKey
) const
{
    /*
     * Alice's and Bob's sifted keys represent the same
     * retained transmission positions.
     *
     * Therefore, their lengths must be identical.
     */
    if (aliceKey.size() != bobKey.size()) {
        throw std::invalid_argument(
            "Alice and Bob sifted keys must have identical sizes"
        );
    }

    /*
     * QBER is undefined when there are no sifted bits.
     *
     * An empty sifted key means that no basis-matched
     * transmission events are available for comparison.
     *
     * This is not equivalent to QBER == 0.0.
     */
    if (aliceKey.empty()) {
        throw std::invalid_argument(
            "Cannot estimate QBER from an empty sifted key"
        );
    }

    /*
     * Validate the classical key material before calculating
     * the QBER.
     *
     * BB84 sifted keys contain binary values only:
     *
     *     0 or 1
     */
    for (const std::uint8_t bit : aliceKey) {
        if (bit > 1U) {
            throw std::invalid_argument(
                "Alice sifted key contains an invalid bit"
            );
        }
    }

    for (const std::uint8_t bit : bobKey) {
        if (bit > 1U) {
            throw std::invalid_argument(
                "Bob sifted key contains an invalid bit"
            );
        }
    }

    QBERResult result{};

    result.comparedBits = aliceKey.size();

    /*
     * Compare only the bits that survived basis sifting.
     *
     * A mismatch represents an observed bit error.
     */
    for (std::size_t index = 0U;
         index < result.comparedBits;
         ++index)
    {
        if (aliceKey[index] != bobKey[index]) {
            ++result.errorBits;
        }
    }

    /*
     * QBER:
     *
     *     errorBits
     *     ----------
     *     comparedBits
     *
     * Convert explicitly to double so that this is floating-point
     * division rather than integer division.
     */
    result.qber =
        static_cast<double>(result.errorBits) /
        static_cast<double>(result.comparedBits);

    return result;
}
