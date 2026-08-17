#include "QuantumChannel.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{

void testEmptyTransmission()
{
    QuantumChannel channel;

    const std::vector<BB84State> states;

    const auto received = channel.transmit(states);

    assert(received.empty());
    assert(channel.transmissionCount() == 0U);
}

void testSingleTransmission()
{
    QuantumChannel channel;

    const std::vector<BB84State> states{
        BB84State::Zero
    };

    const auto received = channel.transmit(states);

    assert(received.size() == 1U);
    assert(received[0] == BB84State::Zero);
    assert(channel.transmissionCount() == 1U);
}

void testAllValidBB84States()
{
    QuantumChannel channel;

    const std::vector<BB84State> states{
        BB84State::Zero,
        BB84State::One,
        BB84State::Plus,
        BB84State::Minus
    };

    const auto received = channel.transmit(states);

    assert(received == states);
    assert(received.size() == states.size());
    assert(channel.transmissionCount() == 4U);
}

void testStateIntegrity()
{
    QuantumChannel channel;

    const std::vector<BB84State> states{
        BB84State::Zero,
        BB84State::One,
        BB84State::Plus,
        BB84State::Minus,
        BB84State::Zero,
        BB84State::Plus
    };

    const auto received = channel.transmit(states);

    /*
     * An ideal quantum channel must not alter the
     * represented BB84 quantum states.
     */
    assert(received == states);
}

void testTransmissionCount()
{
    QuantumChannel channel;

    const std::vector<BB84State> firstTransmission{
        BB84State::Zero,
        BB84State::Plus
    };

    const std::vector<BB84State> secondTransmission{
        BB84State::One,
        BB84State::Minus,
        BB84State::Zero
    };

    const auto firstReceived =
        channel.transmit(firstTransmission);

    assert(firstReceived == firstTransmission);
    assert(channel.transmissionCount() == 2U);

    const auto secondReceived =
        channel.transmit(secondTransmission);

    assert(secondReceived == secondTransmission);
    assert(channel.transmissionCount() == 5U);
}

void testInputIsNotModified()
{
    QuantumChannel channel;

    const std::vector<BB84State> states{
        BB84State::Zero,
        BB84State::Plus,
        BB84State::One,
        BB84State::Minus
    };

    const auto originalStates = states;

    static_cast<void>(channel.transmit(states));

    /*
     * The channel must not modify Alice's transmitted
     * state sequence.
     */
    assert(states == originalStates);
}

void testInvalidState()
{
    QuantumChannel channel;

    /*
     * BB84State is an enum class. Cast an invalid underlying
     * value deliberately to verify defensive validation.
     */
    const auto invalidState =
        static_cast<BB84State>(0xFFU);

    const std::vector<BB84State> states{
        BB84State::Zero,
        invalidState
    };

    bool exceptionThrown = false;

    try
    {
        static_cast<void>(channel.transmit(states));
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);

    /*
     * Invalid transmission must not be counted.
     */
    assert(channel.transmissionCount() == 0U);
}

} // namespace

int main()
{
    testEmptyTransmission();
    testSingleTransmission();
    testAllValidBB84States();
    testStateIntegrity();
    testTransmissionCount();
    testInputIsNotModified();
    testInvalidState();

    std::cout
        << "QuantumChannel tests: PASSED\n";

    return 0;
}