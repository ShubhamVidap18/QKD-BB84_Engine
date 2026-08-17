#include "Alice.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

/*
 * Deterministic RNG for unit testing only.
 */
class DeterministicRandomGenerator final
    : public IRandomGenerator
{
public:
    explicit DeterministicRandomGenerator(
        std::vector<std::uint8_t> data)
        : data_(std::move(data))
    {
    }

    void generateBytes(
        std::span<std::uint8_t> output
    ) override
    {
        if (position_ + output.size() > data_.size()) {
            throw std::runtime_error(
                "Deterministic test RNG exhausted"
            );
        }

        for (std::size_t i = 0; i < output.size(); ++i) {
            output[i] = data_[position_++];
        }
    }

private:
    std::vector<std::uint8_t> data_;
    std::size_t position_{0};
};

void testEncode()
{
    DeterministicRandomGenerator rng({
        0x00
    });

    PhotonGenerator photonGenerator(rng);
    BasisSelector basisSelector(rng);

    Alice alice(
        photonGenerator,
        basisSelector
    );

    const std::vector<std::uint8_t> bits = {
        0,
        1,
        0,
        1
    };

    const std::vector<BB84Basis> bases = {
        BB84Basis::Z,
        BB84Basis::Z,
        BB84Basis::X,
        BB84Basis::X
    };

    const auto transmission =
        alice.encode(bits, bases);

    assert(transmission.size() == 4U);

    assert(
        transmission[0].state ==
        BB84State::Zero
    );

    assert(
        transmission[1].state ==
        BB84State::One
    );

    assert(
        transmission[2].state ==
        BB84State::Plus
    );

    assert(
        transmission[3].state ==
        BB84State::Minus
    );
}

void testMismatchedInputSizes()
{
    DeterministicRandomGenerator rng({
        0x00
    });

    PhotonGenerator photonGenerator(rng);
    BasisSelector basisSelector(rng);

    Alice alice(
        photonGenerator,
        basisSelector
    );

    const std::vector<std::uint8_t> bits = {
        0,
        1
    };

    const std::vector<BB84Basis> bases = {
        BB84Basis::Z
    };

    bool exceptionThrown = false;

    try {
        static_cast<void>(
            alice.encode(bits, bases)
        );
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

int main()
{
    testEncode();
    testMismatchedInputSizes();

    std::cout << "Alice tests passed.\n";

    return 0;
}