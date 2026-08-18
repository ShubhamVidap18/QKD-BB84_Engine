#include "QuantumChannel.hpp"

#include <stdexcept>

std::vector<BB84State> QuantumChannel::transmit(
    const std::vector<BB84State>& states) const
{
    for (const BB84State state : states)
    {
        switch (state)
        {
            case BB84State::Zero:
            case BB84State::One:
            case BB84State::Plus:
            case BB84State::Minus:
                break;

            default:
                throw std::invalid_argument(
                    "QuantumChannel received an invalid BB84 state"
                );
        }
    }

    transmissionCount_ += states.size();

    return states;
}

std::size_t QuantumChannel::transmissionCount() const noexcept
{
    return transmissionCount_;
}
