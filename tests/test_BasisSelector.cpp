#include "BasisSelector.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief Deterministic RNG used only by unit tests.
 *
 * This class must never be used by production BB84 code.
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

void testSelectBasisZ()
{
    /*
     * 0x00 → least significant bit = 0
     *       → Z basis
     */
    DeterministicRandomGenerator rng({
        0x00
    });

    BasisSelector selector(rng);

    assert(
        selector.selectBasis() == BB84Basis::Z
    );
}

void testSelectBasisX()
{
    /*
     * 0x01 → least significant bit = 1
     *       → X basis
     */
    DeterministicRandomGenerator rng({
        0x01
    });

    BasisSelector selector(rng);

    assert(
        selector.selectBasis() == BB84Basis::X
    );
}

void testSelectBases()
{
    /*
     * 0xB2 = binary 10110010
     *
     * The implementation reads bits from LSB to MSB:
     *
     * bit:       0 1 0 0 1 1 0 1
     *
     * Therefore:
     *
     * basis:     Z X Z Z X X Z X
     */
    DeterministicRandomGenerator rng({
        0xB2
    });

    BasisSelector selector(rng);

    const auto bases = selector.selectBases(8);

    const std::vector<BB84Basis> expected = {
        BB84Basis::Z,
        BB84Basis::X,
        BB84Basis::Z,
        BB84Basis::Z,
        BB84Basis::X,
        BB84Basis::X,
        BB84Basis::Z,
        BB84Basis::X
    };

    assert(bases == expected);
}

void testSelectBasesCount()
{
    /*
     * 16 bases require only two random bytes.
     */
    DeterministicRandomGenerator rng({
        0x00,
        0xFF
    });

    BasisSelector selector(rng);

    const auto bases = selector.selectBases(16);

    assert(bases.size() == 16U);
}

void testZeroCount()
{
    DeterministicRandomGenerator rng({});

    BasisSelector selector(rng);

    const auto bases = selector.selectBases(0);

    assert(bases.empty());
}

void testRandomGeneratorFailurePropagates()
{
    /*
     * No data is provided, so the deterministic test RNG
     * will fail when BasisSelector requests randomness.
     */
    DeterministicRandomGenerator rng({});

    BasisSelector selector(rng);

    bool exceptionThrown = false;

    try {
        static_cast<void>(
            selector.selectBasis()
        );
    }
    catch (const std::runtime_error&) {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

int main()
{
    testSelectBasisZ();
    testSelectBasisX();
    testSelectBases();
    testSelectBasesCount();
    testZeroCount();
    testRandomGeneratorFailurePropagates();

    std::cout
        << "BasisSelector tests passed.\n";

    return 0;
}