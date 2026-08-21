#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace bb84
{

class IAuthenticatedClassicalChannel
{
public:

    virtual ~IAuthenticatedClassicalChannel() = default;

    /*
     * Returns the authenticated parity of the supplied bit block.
     *
     * No raw key material is returned.
     */
    [[nodiscard]]
    virtual std::uint8_t queryParity(
        std::span<const std::uint8_t> bits
    ) = 0;

    /*
     * Returns the number of authenticated classical messages
     * exchanged through this channel.
     */
    [[nodiscard]]
    virtual std::size_t messageCount() const noexcept = 0;

    /*
     * Returns the number of key-derived bits disclosed through
     * authenticated parity queries.
     */
    [[nodiscard]]
    virtual std::size_t disclosedBits() const noexcept = 0;
};

} // namespace bb84