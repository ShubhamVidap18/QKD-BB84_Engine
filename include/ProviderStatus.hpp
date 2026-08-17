#pragma once

#include <string_view>

namespace bb84 {

enum class ProviderStatus {
    UNINITIALIZED,
    INITIALIZED,
    READY,
    KEY_GENERATION,
    KEY_AVAILABLE,
    ERROR,
    SHUTDOWN
};

constexpr std::string_view toString(ProviderStatus status) noexcept {
    switch (status) {
        case ProviderStatus::UNINITIALIZED: return "UNINITIALIZED";
        case ProviderStatus::INITIALIZED:   return "INITIALIZED";
        case ProviderStatus::READY:         return "READY";
        case ProviderStatus::KEY_GENERATION:return "KEY_GENERATION";
        case ProviderStatus::KEY_AVAILABLE: return "KEY_AVAILABLE";
        case ProviderStatus::ERROR:         return "ERROR";
        case ProviderStatus::SHUTDOWN:      return "SHUTDOWN";
    }
    return "UNKNOWN";
}

} // namespace bb84
