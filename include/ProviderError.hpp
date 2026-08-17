#pragma once

#include <stdexcept>
#include <string>

namespace bb84 {

enum class ProviderErrorCode {
    INVALID_STATE,
    ALREADY_INITIALIZED,
    CONNECTION_FAILED,
    KEY_NOT_AVAILABLE,
    SHUTDOWN,
    INTERNAL_ERROR
};

class ProviderError final : public std::runtime_error {
public:
    ProviderError(ProviderErrorCode code, const std::string& message)
        : std::runtime_error(message), code_(code) {}

    [[nodiscard]] ProviderErrorCode code() const noexcept { return code_; }

private:
    ProviderErrorCode code_;
};

} // namespace bb84
