#include "Bob.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

class TestRandomGenerator final : public IRandomGenerator {
public:
    explicit TestRandomGenerator(
        std::vector<std::uint8_t> values)
        : values_(std::move(values))
    {
    }

    void generateBytes(
        std::span<std::uint8_t> output) override
    {
        if (index_ + output.size() > values_.size()) {
            throw std::runtime_error(
                "TestRandomGenerator: insufficient test data"
            );
        }

        for (std::size_t i = 0; i < output.size(); ++i) {
            output[i] = values_[index_++];
        }
    }

private:
    std::vector<std::uint8_t> values_;
    std::size_t index_{0};
};


void testMatchingZBasis()
{
    /*
     * Random byte:
     *
     * 0 -> Z basis
     *
     * |0> measured in Z -> 0
     */

    TestRandomGenerator rng({0U});

    Bob bob(rng);

    const std::vector<BB84State> states{
        BB84State::Zero
    };

    const MeasurementBatch result =
        bob.measure(states);

    assert(result.size() == 1U);

    assert(
        result.results[0].basis ==
        BB84Basis::Z
    );

    assert(
        result.results[0].bit == 0U
    );
}


void testMatchingXBasis()
{
    /*
     * Random byte:
     *
     * 1 -> X basis
     *
     * |+> measured in X -> 0
     */

    TestRandomGenerator rng({1U});

    Bob bob(rng);

    const std::vector<BB84State> states{
        BB84State::Plus
    };

    const MeasurementBatch result =
        bob.measure(states);

    assert(result.size() == 1U);

    assert(
        result.results[0].basis ==
        BB84Basis::X
    );

    assert(
        result.results[0].bit == 0U
    );
}


void testMatchingXMinusState()
{
    /*
     * 1 -> X basis
     *
     * |-> measured in X -> 1
     */

    TestRandomGenerator rng({1U});

    Bob bob(rng);

    const std::vector<BB84State> states{
        BB84State::Minus
    };

    const MeasurementBatch result =
        bob.measure(states);

    assert(result.size() == 1U);

    assert(
        result.results[0].basis ==
        BB84Basis::X
    );

    assert(
        result.results[0].bit == 1U
    );
}


void testMismatchedBasisProducesValidBit()
{
    /*
     * |0> measured using X basis.
     *
     * Random data:
     *
     * 1 -> X measurement basis
     * 0 -> measurement result
     */

    TestRandomGenerator rng({
        1U,
        0U
    });

    Bob bob(rng);

    const std::vector<BB84State> states{
        BB84State::Zero
    };

    const MeasurementBatch result =
        bob.measure(states);

    assert(result.size() == 1U);

    assert(
        result.results[0].basis ==
        BB84Basis::X
    );

    assert(
        result.results[0].bit == 0U
    );
}


void testMultipleMeasurements()
{
    /*
     * Four states.
     *
     * Random bytes:
     *
     * 0 -> Z
     * 1 -> X
     * 0 -> Z
     * 1 -> X
     */

    TestRandomGenerator rng({
        0U,
        1U,
        0U,
        1U
    });

    Bob bob(rng);

    const std::vector<BB84State> states{
        BB84State::Zero,
        BB84State::Plus,
        BB84State::One,
        BB84State::Minus
    };

    const MeasurementBatch result =
        bob.measure(states);

    assert(result.size() == 4U);

    assert(
        result.results[0].basis ==
        BB84Basis::Z
    );

    assert(
        result.results[0].bit == 0U
    );

    assert(
        result.results[1].basis ==
        BB84Basis::X
    );

    assert(
        result.results[1].bit == 0U
    );

    assert(
        result.results[2].basis ==
        BB84Basis::Z
    );

    assert(
        result.results[2].bit == 1U
    );

    assert(
        result.results[3].basis ==
        BB84Basis::X
    );

    assert(
        result.results[3].bit == 1U
    );
}


int main()
{
    testMatchingZBasis();
    testMatchingXBasis();
    testMatchingXMinusState();
    testMismatchedBasisProducesValidBit();
    testMultipleMeasurements();

    std::cout << "Bob tests passed.\n";

    return 0;
}