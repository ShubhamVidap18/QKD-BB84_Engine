#include "BB84Provider.hpp"
#include "ProviderError.hpp"

#include <cassert>
#include <iostream>

using namespace bb84;

int main() {
    BB84Provider provider;

    assert(provider.getStatus() == ProviderStatus::UNINITIALIZED);

    provider.initialize();
    assert(provider.getStatus() == ProviderStatus::INITIALIZED);

    provider.openConnection();
    assert(provider.getStatus() == ProviderStatus::READY);

    const auto key = provider.getKey("test-key-001");
    assert(!key.empty());
    assert(provider.getStatus() == ProviderStatus::KEY_AVAILABLE);

    const auto same_key = provider.getKey("test-key-001");
    assert(key == same_key);

    provider.closeConnection();
    assert(provider.getStatus() == ProviderStatus::INITIALIZED);

    provider.shutdown();
    assert(provider.getStatus() == ProviderStatus::SHUTDOWN);

    // shutdown() is intentionally idempotent.
    provider.shutdown();
    assert(provider.getStatus() == ProviderStatus::SHUTDOWN);

    bool rejected = false;
    try {
        provider.initialize();
    } catch (const ProviderError& error) {
        rejected = error.code() == ProviderErrorCode::SHUTDOWN;
    }
    assert(rejected);

    std::cout << "BB84 provider state-machine tests passed.\n";
    return 0;
}
