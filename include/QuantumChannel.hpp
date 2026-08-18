#pragma once

#include "PhotonGenerator.hpp"

#include <cstddef>
#include <vector>

/**
 * @brief Represents the quantum communication channel between
 *        Alice and Bob in the BB84 protocol.
 *
 * The QuantumChannel is responsible only for transporting
 * prepared BB84 quantum states from Alice to Bob.
 *
 * Current implementation:
 *  - Ideal/noiseless channel
 *  - No state transformation
 *  - No eavesdropper
 *  - No attenuation
 *  - No photon loss
 *
 * Future implementations may introduce configurable channel
 * characteristics such as attenuation, loss, polarization
 * errors, noise, or eavesdropping models.
 *
 * Security-sensitive classical information such as Alice's
 * raw bits and preparation bases must never pass through this
 * interface.
 */
class QuantumChannel {
public:

    /**
     * @brief Construct an ideal BB84 quantum channel.
     */
    QuantumChannel() = default;

    QuantumChannel(const QuantumChannel&) = delete;
    QuantumChannel& operator=(const QuantumChannel&) = delete;

    QuantumChannel(QuantumChannel&&) = delete;
    QuantumChannel& operator=(QuantumChannel&&) = delete;

    ~QuantumChannel() = default;

    /**
     * @brief Transmit prepared BB84 states from Alice to Bob.
     *
     * In the current ideal-channel implementation, every
     * transmitted state arrives unchanged.
     *
     * @param states BB84 states prepared by Alice.
     *
     * @return States received by Bob.
     *
     * @throws std::invalid_argument if an invalid BB84 state
     *         is encountered.
     */
    [[nodiscard]]
    std::vector<BB84State> transmit(
        const std::vector<BB84State>& states
    ) const;

    /**
     * @brief Return the number of states transmitted through
     *        the channel.
     */
    [[nodiscard]]
    std::size_t transmissionCount() const noexcept;

    private:
    mutable std::size_t transmissionCount_{0U};
};
