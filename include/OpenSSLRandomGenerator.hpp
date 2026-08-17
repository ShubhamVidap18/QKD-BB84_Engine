#pragma once

#include "IRandomGenerator.hpp"

#include <cstdint>
#include <span>

/**
 * @brief Production CSPRNG implementation backed by OpenSSL.
 *
 * Uses OpenSSL's RAND_priv_bytes_ex() API to obtain cryptographically
 * secure random bytes suitable for security-sensitive protocol values.
 */
class OpenSSLRandomGenerator final : public IRandomGenerator {
public:
    /**
     * @brief Construct an OpenSSL-backed random generator.
     *
     * @param securityStrength Requested security strength in bits.
     *
     * 256 is used by default and is appropriate for this application.
     */
    explicit OpenSSLRandomGenerator(
        unsigned int securityStrength = 256
    );

    ~OpenSSLRandomGenerator() override = default;

    OpenSSLRandomGenerator(const OpenSSLRandomGenerator&) = delete;
    OpenSSLRandomGenerator& operator=(
        const OpenSSLRandomGenerator&
    ) = delete;

    OpenSSLRandomGenerator(OpenSSLRandomGenerator&&) = delete;
    OpenSSLRandomGenerator& operator=(
        OpenSSLRandomGenerator&&
    ) = delete;

    /**
     * @brief Generate cryptographically secure random bytes.
     *
     * @param output Buffer that will receive random bytes.
     *
     * @throws std::invalid_argument if output is invalid.
     * @throws std::runtime_error if OpenSSL fails to generate randomness.
     */
    void generateBytes(
        std::span<std::uint8_t> output
    ) override;

private:
    unsigned int securityStrength_;
};