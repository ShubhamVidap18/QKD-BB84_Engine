#include "AuthenticatedClassicalChannel.hpp"

#include <stdexcept>

namespace bb84
{

std::uint8_t AuthenticatedClassicalChannel::queryParity(
    std::span<const std::uint8_t> bits)
{
    std::uint8_t parity = 0U;

    for (const std::uint8_t bit : bits)
    {
        if (bit > 1U)
        {
            throw std::invalid_argument(
                "Authenticated classical channel received "
                "a non-binary bit"
            );
        }

        parity ^= bit;
    }

    /*
     * One authenticated parity value was disclosed.
     */
    ++messageCount_;
    ++disclosedBits_;

    return parity;
}

std::size_t
AuthenticatedClassicalChannel::messageCount() const noexcept
{
    return messageCount_;
}

std::size_t
AuthenticatedClassicalChannel::disclosedBits() const noexcept
{
    return disclosedBits_;
}

} // namespace bb84