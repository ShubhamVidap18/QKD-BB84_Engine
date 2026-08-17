#include "OpenSSLRandomGenerator.hpp"

#include <openssl/rand.h>

#include <limits>
#include <stdexcept>

OpenSSLRandomGenerator::OpenSSLRandomGenerator(
    unsigned int securityStrength)
    : securityStrength_(securityStrength)
{
    if (securityStrength_ == 0U) {
        throw std::invalid_argument(
            "Security strength must be greater than zero"
        );
    }
}

void OpenSSLRandomGenerator::generateBytes(
    std::span<std::uint8_t> output)
{
    if (output.empty()) {
        return;
    }

    /*
     * RAND_priv_bytes_ex() accepts size_t for the requested number
     * of bytes. No narrowing conversion is therefore required here.
     *
     * OpenSSL returns:
     *   1  -> success
     *   0  -> failure
     *  -1  -> unsupported
     */
    const int result = RAND_priv_bytes_ex(
        nullptr,
        output.data(),
        output.size(),
        securityStrength_
    );

    if (result != 1) {
        /*
         * Do not fall back to std::random_device, rand(), time(),
         * or another non-CSPRNG source.
         *
         * Failure of the CSPRNG is a security failure and must
         * propagate to the caller.
         */
        throw std::runtime_error(
            "OpenSSL CSPRNG failed to generate secure randomness"
        );
    }
}