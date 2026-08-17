#include "Alice.hpp"

#include <stdexcept>
#include <utility>

Alice::Alice(
    PhotonGenerator& photonGenerator,
    BasisSelector& basisSelector
)
    : photonGenerator_(photonGenerator),
      basisSelector_(basisSelector)
{
}

std::vector<std::uint8_t>
Alice::generateRawBits(std::size_t count)
{
    return photonGenerator_.generateBits(count);
}

std::vector<BB84Basis>
Alice::selectBases(std::size_t count)
{
    return basisSelector_.selectBases(count);
}

std::vector<AliceTransmission>
Alice::encode(
    const std::vector<std::uint8_t>& bits,
    const std::vector<BB84Basis>& bases
)
{
    if (bits.size() != bases.size()) {
        throw std::invalid_argument(
            "Alice bits and bases must have identical sizes"
        );
    }

    std::vector<AliceTransmission> transmission;
    transmission.reserve(bits.size());

    for (std::size_t i = 0; i < bits.size(); ++i) {

        const BB84State state =
            photonGenerator_.generateQuantumState(
                bits[i],
                bases[i]
            );

        transmission.push_back(
            AliceTransmission{
                bits[i],
                bases[i],
                state
            }
        );
    }

    return transmission;
}

std::vector<AliceTransmission>
Alice::prepareTransmission(std::size_t count)
{
    const auto bits = generateRawBits(count);

    const auto bases = selectBases(count);

    return encode(bits, bases);
}