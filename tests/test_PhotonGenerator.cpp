#include "PhotonGenerator.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

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

void testGenerateBit()
{
    DeterministicRandomGenerator rng({
        0x00,
        0x01
    });

    PhotonGenerator generator(rng);

    assert(generator.generateBit() == 0U);
    assert(generator.generateBit() == 1U);
}

void testGenerateBits()
{
    /*
     * 0xB2 = 10110010
     *
     * Our implementation reads bits from LSB to MSB:
     *
     * 0 1 0 0 1 1 0 1
     */
    DeterministicRandomGenerator rng({
        0xB2
    });

    PhotonGenerator generator(rng);

    const auto bits = generator.generateBits(8);

    const std::vector<std::uint8_t> expected = {
        0, 1, 0, 0, 1, 1, 0, 1
    };

    assert(bits == expected);
}

void testQuantumStateMapping()
{
    /*
     * No randomness is involved in this part.
     */

    DeterministicRandomGenerator rng({});

    PhotonGenerator generator(rng);

    assert(
        generator.generateQuantumState(
            0U,
            BB84Basis::Z
        ) == BB84State::Zero
    );

    assert(
        generator.generateQuantumState(
            1U,
            BB84Basis::Z
        ) == BB84State::One
    );

    assert(
        generator.generateQuantumState(
            0U,
            BB84Basis::X
        ) == BB84State::Plus
    );

    assert(
        generator.generateQuantumState(
            1U,
            BB84Basis::X
        ) == BB84State::Minus
    );
}

void testInvalidBit()
{
    DeterministicRandomGenerator rng({});

    PhotonGenerator generator(rng);

    bool exceptionThrown = false;

    try {
        static_cast<void>(
            generator.generateQuantumState(
                2U,
                BB84Basis::Z
            )
        );
    }
    catch (const std::invalid_argument&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

int main()
{
    testGenerateBit();
    testGenerateBits();
    testQuantumStateMapping();
    testInvalidBit();

    std::cout
        << "PhotonGenerator tests passed.\n";

    return 0;
}