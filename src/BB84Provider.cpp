#include "BB84Provider.hpp"
#include "ProviderError.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <utility>

namespace bb84 {

BB84Provider::BB84Provider(BB84Config config)
    : config_(std::move(config)) {
    if (config_.raw_bit_count == 0 ||
        config_.minimum_key_bytes == 0 ||
        config_.qber_threshold < 0.0 ||
        config_.qber_threshold > 1.0) {
        throw std::invalid_argument("Invalid BB84 configuration");
    }
}

BB84Provider::~BB84Provider() {
    shutdown();
}

void BB84Provider::initialize() {
    std::lock_guard lock(mutex_);

    if (status_ == ProviderStatus::SHUTDOWN) {
        throw ProviderError(
            ProviderErrorCode::SHUTDOWN,
            "Provider has been shut down and cannot be reinitialized");
    }

    if (status_ != ProviderStatus::UNINITIALIZED) {
        throw ProviderError(
            ProviderErrorCode::ALREADY_INITIALIZED,
            "Provider is already initialized");
    }

    // Initialization only establishes provider resources/configuration.
    // Quantum/classical channels are opened separately.
    status_ = ProviderStatus::INITIALIZED;
}

void BB84Provider::openConnection() {
    std::lock_guard lock(mutex_);

    requireStateLocked(ProviderStatus::INITIALIZED);

    if (config_.require_authenticated_classical_channel) {
        // Placeholder for the production authenticated classical-channel
        // handshake. Do not silently proceed when authentication is required.
        // The actual channel implementation will be injected later.
    }

    status_ = ProviderStatus::READY;
}

ByteVector BB84Provider::getKey(const KeyId& key_id) {
    if (key_id.empty()) {
        throw std::invalid_argument("key_id must not be empty");
    }

    {
        std::lock_guard lock(mutex_);

        if (status_ == ProviderStatus::KEY_AVAILABLE &&
            active_key_id_.has_value() &&
            *active_key_id_ == key_id) {
            // Return a copy. The provider never exposes its internal storage.
            return active_key_;
        }

        if (status_ != ProviderStatus::READY) {
            throw ProviderError(
                ProviderErrorCode::INVALID_STATE,
                "getKey() is only available from READY or KEY_AVAILABLE");
        }

        // The actual implementation will first query KeyManager/KMS here.
        // If no valid key exists, key generation is started internally.
        //status_ = ProviderStatus::KEY_GENERATION;
    }

    try {
        return generateKeyInternal(key_id);
    } catch (...) {
        std::lock_guard lock(mutex_);
        active_key_.clear();
        active_key_.shrink_to_fit();
        active_key_id_.reset();
        status_ = ProviderStatus::ERROR;
        throw;
    }
}

ByteVector BB84Provider::generateKeyInternal(const KeyId& key_id) {
    // This is deliberately a minimal state-machine implementation.
    // QuantumState generation, channel transport, Bob measurement,
    // sifting, QBER, Cascade, privacy amplification and KeyManager
    // are implemented in their own modules in subsequent steps.

    // Temporary cryptographic placeholder: use std::random_device only
    // for scaffolding. Production key material MUST come from the complete
    // BB84 + authenticated-classical-channel + privacy-amplification path.
    //
    // This function therefore MUST NOT be treated as a production key
    // generator yet.
    std::vector<std::uint8_t> key(config_.minimum_key_bytes);

    std::random_device rd;
    for (auto& byte : key) {
        byte = static_cast<std::uint8_t>(rd());
    }

    {
        std::lock_guard lock(mutex_);

        // A failed/aborted session must never publish partial key material.
        if (status_ != ProviderStatus::READY) {
            throw ProviderError(
                ProviderErrorCode::INVALID_STATE,
                "Key generation state changed unexpectedly");
        }

        active_key_id_ = key_id;
        active_key_ = key;

        // Placeholder until QBER and key-rate modules are connected.
        qber_ = 0.0;
        key_rate_ = 0.0;

        status_ = ProviderStatus::KEY_AVAILABLE;
    }

    return key;
}

void BB84Provider::closeConnection() {
    std::lock_guard lock(mutex_);

    if (status_ != ProviderStatus::READY &&
        status_ != ProviderStatus::KEY_AVAILABLE) {
        throw ProviderError(
            ProviderErrorCode::INVALID_STATE,
            "closeConnection() is only valid in READY or KEY_AVAILABLE");
    }

    // Key lifecycle remains under KeyManager/KMS ownership. This call only
    // closes the provider's active transport/session connection.
    status_ = ProviderStatus::INITIALIZED;
}

ProviderStatus BB84Provider::getStatus() const noexcept {
    std::lock_guard lock(mutex_);
    return status_;
}

double BB84Provider::getQBER() const noexcept {
    std::lock_guard lock(mutex_);
    return qber_;
}

double BB84Provider::getKeyRate() const noexcept {
    std::lock_guard lock(mutex_);
    return key_rate_;
}

void BB84Provider::shutdown() noexcept {
    std::lock_guard lock(mutex_);

    // Idempotent cleanup.
    if (status_ == ProviderStatus::SHUTDOWN) {
        return;
    }

    // Do not retain sensitive material after shutdown.
    std::fill(active_key_.begin(), active_key_.end(), std::uint8_t{0});
    active_key_.clear();
    active_key_.shrink_to_fit();
    active_key_id_.reset();

    qber_ = 0.0;
    key_rate_ = 0.0;

    status_ = ProviderStatus::SHUTDOWN;
}

bool BB84Provider::transitionLocked(
    ProviderStatus expected,
    ProviderStatus next) noexcept {
    if (status_ != expected) {
        return false;
    }
    status_ = next;
    return true;
}

void BB84Provider::requireStateLocked(ProviderStatus expected) const {
    if (status_ != expected) {
        throw ProviderError(
            ProviderErrorCode::INVALID_STATE,
            "Operation is not valid in the current provider state");
    }
}

} // namespace bb84
