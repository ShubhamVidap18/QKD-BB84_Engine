#include "BB84Provider.hpp"
#include "ProviderStatus.hpp"

#include <iostream>

int main() {
    bb84::BB84Provider provider;

    provider.initialize();
    provider.openConnection();

    std::cout << "State: "
              << bb84::toString(provider.getStatus()) << '\n';

    // In the final implementation, getKey() will use KeyManager/KMS first
    // and invoke the BB84 generation pipeline only when policy requires it.
    const auto key = provider.getKey("demo-key-001");

    std::cout << "Key acquired: " << (!key.empty() ? "yes" : "no") << '\n';
    std::cout << "QBER: " << provider.getQBER() << '\n';
    std::cout << "Key rate: " << provider.getKeyRate() << '\n';
    provider.shutdown();
}
