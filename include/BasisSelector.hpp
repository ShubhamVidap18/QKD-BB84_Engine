#pragma once

#include "PhotonGenerator.hpp"
#include "IRandomGenerator.hpp"

#include <cstddef>
#include <vector>

/**
 * @brief Selects random BB84 measurement/preparation bases.
 *
 * BasisSelector is responsible only for basis selection.
 * It does not perform encoding, measurement, sifting, or
 * any other BB84 protocol operation.
 *
 * Randomness is provided through IRandomGenerator so that
 * production code can use the OpenSSL CSPRNG while tests
 * can inject deterministic randomness.
 */
class BasisSelector {
public:
    /**
     * @brief Construct a BasisSelector.
     *
     * @param randomGenerator Randomness provider.
     *
     * The referenced generator must remain valid for the
     * lifetime of this BasisSelector.
     */
    explicit BasisSelector(
        IRandomGenerator& randomGenerator
    );

    ~BasisSelector() = default;

    BasisSelector(const BasisSelector&) = delete;
    BasisSelector& operator=(
        const BasisSelector&
    ) = delete;

    BasisSelector(BasisSelector&&) = delete;
    BasisSelector& operator=(
        BasisSelector&&
    ) = delete;

    /**
     * @brief Select one random BB84 basis.
     *
     * @return BB84Basis::Z or BB84Basis::X.
     *
     * @throws std::runtime_error if the secure random
     *         generator fails.
     */
    BB84Basis selectBasis();

    /**
     * @brief Select multiple independent BB84 bases.
     *
     * @param count Number of bases required.
     *
     * @return Vector containing exactly count bases.
     *
     * @throws std::runtime_error if the secure random
     *         generator fails.
     */
    std::vector<BB84Basis> selectBases(
        std::size_t count
    );

private:
    IRandomGenerator& randomGenerator_;
};