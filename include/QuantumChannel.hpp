#pragma once

#include "PhotonGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

/**
 * @brief Represents the quantum communication channel between
 *        Alice and Bob in the BB84 protocol.
 *
 * The channel transports prepared BB84 states.
 *
 */
class QuantumChannel
{
public:

    
    QuantumChannel() = default;

    QuantumChannel(const QuantumChannel&) = delete;
    QuantumChannel& operator=(const QuantumChannel&) = delete;

    QuantumChannel(QuantumChannel&&) = delete;
    QuantumChannel& operator=(QuantumChannel&&) = delete;

    ~QuantumChannel() = default;


    /**
     * @brief Transmit prepared BB84 states from Alice to Bob.
     *
     * The default channel returns the states unchanged.
     *
     * In test mode, controlled channel errors may transform
     * BB84 states before they reach Bob.
     *
     * @param states BB84 states prepared by Alice.
     *
     * @return States received by Bob.
     *
     * @throws std::invalid_argument if an invalid state or
     *         channel configuration is encountered.
     */
    [[nodiscard]]
    std::vector<BB84State> transmit(
        const std::vector<BB84State>& states
    ) const;


    /**
     * @brief Return the number of states transmitted.
     */
    [[nodiscard]]
    std::size_t transmissionCount() const noexcept;

private:
    mutable std::size_t transmissionCount_{0U};
};
