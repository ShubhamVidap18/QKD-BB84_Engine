#pragma once

#include "ProviderStatus.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace bb84 {

using KeyId = std::string;
using ByteVector = std::vector<std::uint8_t>;

class IQKDProvider {
public:
    virtual ~IQKDProvider() = default;

    virtual void initialize() = 0;
    virtual void openConnection() = 0;

    // Returns an existing usable key or obtains/generates one according
    // to the provider's configured key-management policy.
    virtual ByteVector getKey(const KeyId& key_id) = 0;

    virtual void closeConnection() = 0;

    [[nodiscard]] virtual ProviderStatus getStatus() const noexcept = 0;
    [[nodiscard]] virtual double getQBER() const noexcept = 0;
    [[nodiscard]] virtual double getKeyRate() const noexcept = 0;

    // Must be safe to call repeatedly and from cleanup paths.
    virtual void shutdown() noexcept = 0;
};

} 
