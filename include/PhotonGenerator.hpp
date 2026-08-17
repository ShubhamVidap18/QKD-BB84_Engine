#pragma once

#include "IRandomGenerator.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

enum class BB84Basis : std::uint8_t {
    Z = 0,
    X = 1
};

enum class BB84State : std::uint8_t {
    Zero = 0,   // |0>
    One = 1,    // |1>
    Plus = 2,   // |+>
    Minus = 3   // |->
};

/**
 * @brief Generates BB84 protocol bits and maps them to BB84 states.
 *
 * PhotonGenerator does not own the randomness implementation.
 * Randomness is supplied through IRandomGenerator.
 */
class PhotonGenerator {
public:
    /**
     * @brief Construct a PhotonGenerator using the supplied RNG.
     *
     * @param randomGenerator Secure randomness provider.
     *
     * The referenced generator must outlive this PhotonGenerator.
     */
    explicit PhotonGenerator(
        IRandomGenerator& randomGenerator
    );

    ~PhotonGenerator() = default;

    PhotonGenerator(const PhotonGenerator&) = delete;
    PhotonGenerator& operator=(
        const PhotonGenerator&
    ) = delete;

    PhotonGenerator(PhotonGenerator&&) = delete;
    PhotonGenerator& operator=(
        PhotonGenerator&&
    ) = delete;

    /**
     * @brief Generate one cryptographically secure random bit.
     *
     * @return 0 or 1.
     */
    std::uint8_t generateBit();

    /**
     * @brief Generate multiple cryptographically secure random bits.
     *
     * @param count Number of bits required.
     *
     * @return Vector containing exactly count bits.
     */
    std::vector<std::uint8_t> generateBits(
        std::size_t count
    );

    /**
     * @brief Convert a bit and basis into a BB84 quantum state.
     *
     * This operation itself does not require randomness.
     *
     * @param bit Must be 0 or 1.
     * @param basis BB84 Z or X basis.
     *
     * @return Corresponding BB84 state.
     */
    BB84State generateQuantumState(
        std::uint8_t bit,
        BB84Basis basis
    );

private:
    IRandomGenerator& randomGenerator_;
};