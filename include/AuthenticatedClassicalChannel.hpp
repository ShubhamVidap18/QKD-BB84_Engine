#pragma once

#include "ErrorReconciliation.hpp"
#include "IAuthenticatedClassicalChannel.hpp"

#include <cstdint>
#include <queue>
#include <vector>

namespace bb84
{
class AuthenticatedClassicalChannel final
    : public IAuthenticatedClassicalChannel
{
public:

    AuthenticatedClassicalChannel() = default;

    [[nodiscard]]
    std::uint8_t queryParity(
        std::span<const std::uint8_t> bits
    ) override;

    [[nodiscard]]
    std::size_t messageCount() const noexcept override;

    [[nodiscard]]
    std::size_t disclosedBits() const noexcept override;

private:

    std::size_t messageCount_{0U};

    std::size_t disclosedBits_{0U};
};

} // namespace bb84