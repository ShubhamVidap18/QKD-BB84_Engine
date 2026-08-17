Build and Execute

From the root directory of the repository, configure and build the project using CMake with the Ninja build system.

1. Configure the project
cmake -S . -B build -G Ninja

This configures the project and generates the Ninja build files inside the build/ directory.

2. Build the project
cmake --build build

This compiles the BB84 engine library, examples, and test executables.

3. Run the tests
ctest --test-dir build --output-on-failure

This executes the registered test cases and displays detailed output for any failed tests.

4. Run examples

After a successful build, example executables are available inside the build/ directory.

For example:

./build/alice_example.exe
./build/bob_example.exe
./build/basis_sifter_example.exe
./build/bb84_state_machine_example.exe

On Linux, the executable names may not contain the .exe extension.

Build Summary

The complete basic build sequence from the repository root is:

cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
Requirements

The project currently requires:

C++20-compatible compiler
CMake
Ninja
OpenSSL
CTest

The project is currently being developed and tested using MSYS2 UCRT64 on Windows.

------------------------------------------------------------------------------------
# BB84 Engine

A modular C++20 implementation of the **BB84 Quantum Key Distribution (QKD)** protocol, designed around an `IQKDProvider` / `BB84Provider` architecture.

The project currently implements the foundational BB84 components required to model the protocol from **random bit and basis generation through Alice's state preparation, Bob's measurement, and classical basis sifting**. It also provides interfaces and component placeholders for later stages such as QBER estimation, Cascade error correction, privacy amplification, and key management.

> **Current implementation status:** The BB84 preparation, CSPRNG-based randomness, Bob measurement, and basis-sifting path are implemented and tested. The complete production-grade QKD pipeline is still under development.

---

## 1. Project Overview

The purpose of `bb84-engine` is to provide a clean, modular implementation of the BB84 protocol that can eventually be integrated behind a provider-level API.

The architecture separates:

* Provider/API orchestration
* Alice-side BB84 preparation
* Cryptographically secure randomness
* BB84 basis selection
* Quantum-state representation
* Bob-side measurement
* Basis reconciliation / sifting
* Quantum-channel abstraction
* Classical-channel abstraction
* QBER estimation
* Error correction
* Privacy amplification
* Key management

The project is intentionally structured so that individual BB84 components can be developed, tested, and validated independently before being connected into a complete production QKD pipeline.

---

## 2. Current BB84 Flow

The currently implemented BB84 path is:

```text
                    BB84 ENGINE
                         |
                         v
                  OpenSSL CSPRNG
                         |
             +-----------+-----------+
             |                       |
             v                       v
          Alice                    Bob
             |                       |
     +-------+-------+       +-------+-------+
     |               |       |               |
 Random raw bits  Random   Random basis   Measurement
                  bases        |               |
     |               |         |               |
     +-------+-------+         +-------+-------+
             |                         |
             v                         v
       BB84 quantum states      Measurement results
             |                         |
             +----------+--------------+
                        |
                        v
                   Basis Sifting
                        |
                        v
                   Sifted Key
```

The important property is that Alice and Bob independently select their bases using the project's CSPRNG-backed random generator.

`BasisSifter` itself is deterministic. It does not generate randomness.

---

## 3. BB84 Protocol Model

The implementation models the four standard BB84 quantum states:

| Basis | Bit | State |     |
| ----- | --: | ----- | --- |
| Z     |   0 | `     | 0⟩` |
| Z     |   1 | `     | 1⟩` |
| X     |   0 | `     | +⟩` |
| X     |   1 | `     | −⟩` |

The basis representation is provided by:

```cpp
enum class BB84Basis
```

and the quantum-state representation is provided by:

```cpp
enum class BB84State
```

These definitions are currently associated with `PhotonGenerator.hpp`.

---

## 4. Cryptographically Secure Randomness

Randomness is provided through an abstraction:

```text
IRandomGenerator
        |
        v
OpenSSLRandomGenerator
```

The main random-generation interface is:

```cpp
generateBytes(...)
```

The BB84 components receive the random generator through dependency injection rather than constructing their own random-number generator internally.

This allows the same CSPRNG implementation to be used by multiple protocol components.

### Current CSPRNG consumers

The current BB84 implementation uses the random generator for:

* Alice's raw bit generation
* Alice's basis selection
* Bob's measurement-basis selection
* Probabilistic measurement when incompatible bases are used

The implementation uses `OpenSSLRandomGenerator` as the concrete random generator.

This avoids relying on ordinary non-cryptographic PRNGs such as:

```text
rand()
std::rand()
std::mt19937
std::mt19937_64
```

for BB84 protocol randomness.

---

# 5. Alice

Alice is responsible for preparing the BB84 transmission.

Implemented responsibilities include:

1. Generate random raw bits.
2. Select independent random preparation bases.
3. Encode each bit/basis pair into a BB84 quantum state.
4. Produce the resulting transmission sequence.

The main abstraction is:

```cpp
class Alice
```

with the following conceptual flow:

```text
Random bits
     +
Random bases
     |
     v
Alice::encode()
     |
     v
BB84State
```

An individual transmission is represented by:

```cpp
struct AliceTransmission
```

which contains:

```text
rawBit
basis
state
```

Alice does not perform:

* Bob's measurement
* basis sifting
* QBER estimation
* error correction
* privacy amplification
* key storage

---

# 6. Bob

Bob represents the receiving party.

Bob is responsible for:

1. Selecting a random measurement basis.
2. Measuring the received BB84 state.
3. Producing the measurement result.

The main abstraction is:

```cpp
class Bob
```

The measurement result is represented by:

```cpp
struct MeasurementResult
```

containing:

```text
bit
basis
```

Multiple results are represented by:

```cpp
struct MeasurementBatch
```

Bob does not have access to Alice's original raw bits or preparation bases during measurement.

---

# 7. Quantum States

The project currently models BB84 states logically rather than implementing physical photon transmission.

The `PhotonGenerator` component converts:

```text
classical bit + basis
```

into the corresponding:

```text
BB84State
```

For example:

```text
bit = 0, basis = Z
        |
        v
    BB84State::Zero
```

and:

```text
bit = 1, basis = X
        |
        v
    BB84State::Minus
```

This is a software-level BB84 state model.

It should not be interpreted as a physical optical implementation.

---

# 8. Quantum Channel

The project contains a:

```cpp
QuantumChannel
```

component and corresponding:

```text
include/QuantumChannel.hpp
src/QuantumChannel.cpp
```

The quantum-channel layer is intended to represent the communication boundary between Alice and Bob.

However, the current basis-sifting example does not yet model a complete physical quantum communication channel with realistic:

* photon loss
* channel noise
* attenuation
* detector imperfections
* eavesdropping
* Eve
* physical optical transmission

The current BB84 state representation is a software abstraction.

Therefore, the project should currently be understood as a **logical BB84 protocol engine**, not a physical QKD simulator.

---

# 9. Basis Sifting

Basis sifting is the current classical reconciliation stage.

The `BasisSifter` receives:

```text
Alice raw bits
Alice preparation bases
Bob measurement bits
Bob measurement bases
```

and compares the bases for every transmission.

The rule is:

```text
Alice basis == Bob basis
        |
        +----> RETAIN

Alice basis != Bob basis
        |
        +----> DISCARD
```

The result is represented by:

```cpp
BasisSiftResult
```

which contains information such as:

* retained transmission indices
* Alice's sifted key
* Bob's sifted key
* retained count
* discarded count

The implementation also validates:

* input vector sizes
* binary bit values
* matching transmission counts

---

## 10. Example Sifting Output

A typical execution may produce output similar to:

```text
Transmission states : 8

Alice raw bits: 00101010
Alice bases: Z X X X Z X Z Z
Bob measurements: 00101010
Bob bases: Z X Z X X Z Z X

Performing BB84 basis reconciliation...

Index   A-Bit     A-Basis   B-Bit     B-Basis   Decision
------------------------------------------------------------
0       0         Z         0         Z         RETAIN
1       0         X         0         X         RETAIN
2       1         X         1         Z         DISCARD
3       0         X         0         X         RETAIN
4       1         Z         1         X         DISCARD
5       0         X         0         Z         DISCARD
6       1         Z         1         Z         RETAIN
7       0         Z         0         X         DISCARD
```

The exact output changes between executions because Alice and Bob use independent CSPRNG-based random choices.

For an 8-event transmission, the number of retained and discarded events is therefore **not fixed**.

For example:

```text
Retained = 4
Discarded = 4
```

is valid, but so are other distributions.

---

# 11. Sifting Validation

The example validates protocol invariants rather than expecting a fixed random result.

The validation checks that:

```text
retained + discarded = total transmissions
```

and:

```text
matching indices = retained events
```

and:

```text
Alice sifted key length = retained events
Bob sifted key length = retained events
```

Finally:

```text
Alice sifted key == Bob sifted key
```

is checked for the ideal channel implementation.

This is preferable to hardcoding a result such as:

```text
retained = 5
discarded = 3
```

because basis selection is intentionally random.

---

# 12. Classical Channel

The project contains a classical-channel abstraction:

```cpp
IClassicalChannel
```

and its implementation:

```text
src/ClassicalChannel.cpp
```

The classical channel is intended to represent authenticated classical communication required during later BB84 stages.

The classical channel is separate from the quantum-state path.

In a complete BB84 implementation, it can be used for operations such as:

* basis reconciliation
* parameter estimation
* error-correction communication
* privacy-amplification coordination

The current implementation is still being developed toward the complete protocol pipeline.

---

# 13. QBER

The project contains a QBER component:

```text
include/QBER.hpp
src/QBER.cpp
```

QBER stands for:

**Quantum Bit Error Rate**

Conceptually:

```text
QBER =
number of mismatched retained bits
----------------------------------
number of compared retained bits
```

QBER is an important BB84 security parameter because an elevated error rate can indicate:

* channel noise
* implementation errors
* eavesdropping
* other disturbances

The QBER stage is part of the planned complete BB84 pipeline.

---

# 14. Cascade Error Correction

The project contains:

```text
include/Cascade.hpp
src/Cascade.cpp
```

Cascade is intended to provide classical error reconciliation between Alice and Bob after sifting and QBER estimation.

Conceptually:

```text
Sifted keys
     |
     v
QBER estimation
     |
     v
Cascade
     |
     v
Corrected shared key
```

The complete production-grade Cascade implementation is still under development.

---

# 15. Privacy Amplification

The project contains:

```text
include/PrivacyAmplification.hpp
src/PrivacyAmplification.cpp
```

Privacy amplification is intended to reduce any information potentially available to an adversary about the reconciled key.

Conceptually:

```text
Reconciled key
      |
      v
Privacy Amplification
      |
      v
Final secret key
```

This stage is required before treating the resulting key as a secure QKD key.

The complete production-grade privacy-amplification implementation is still under development.

---

# 16. Key Management

The project contains:

```text
include/KeyManager.hpp
src/KeyManager.cpp
```

The `KeyManager` is intended to provide the boundary between the BB84 protocol engine and key storage/management infrastructure.

The design aims to ensure that raw intermediate BB84 data is not unnecessarily exposed through the public provider API.

The final integration with an external KMS or production key-management infrastructure is still under development.

---

# 17. Provider Architecture

The public provider architecture is represented by:

```text
IQKDProvider
     |
     v
BB84Provider
```

The purpose of this layer is to expose a stable application-facing interface while keeping the internal BB84 protocol stages modular.

The conceptual architecture is:

```text
Application
     |
     v
IQKDProvider
     |
     v
BB84Provider
     |
     +----------------------+
     |                      |
     v                      v
Key Management        BB84 Pipeline
                           |
                           +--> Alice
                           |
                           +--> Quantum Channel
                           |
                           +--> Bob
                           |
                           +--> Classical Channel
                           |
                           +--> Sifting
                           |
                           +--> QBER
                           |
                           +--> Cascade
                           |
                           +--> Privacy Amplification
                           |
                           +--> Key Manager
```

---

# 18. Provider State Machine

The project also contains provider lifecycle/state-machine support.

Relevant files include:

```text
include/ProviderStatus.hpp
include/ProviderError.hpp
include/BB84Provider.hpp
src/BB84Provider.cpp
docs/state-machine.md
examples/state_machine_example.cpp
tests/test_bb84_state_machine.cpp
```

The state-machine implementation provides the foundation for controlling the provider lifecycle and orchestration behavior independently of the underlying BB84 protocol stages.

See:

```text
docs/state-machine.md
```

for the current state-machine documentation.

---

# 19. Public API Design

The public API is based on:

```cpp
IQKDProvider
```

with:

```cpp
BB84Provider
```

as the BB84-specific implementation.

The architecture is designed to prevent application code from directly interacting with sensitive intermediate protocol data.

In particular, the public provider interface should not expose:

* Alice's raw bits
* Alice's preparation bases
* Bob's measurement results
* intermediate sifted data
* partial keys
* internal protocol state

These values should remain within the appropriate protocol components.

---

# 20. Directory Structure

```text
bb84-engine/
├── CMakeLists.txt
├── README.md
│
├── docs/
│   └── state-machine.md
│
├── examples/
│   ├── alice_example.cpp
│   ├── basis_sifter_example.cpp
│   ├── bob_example.cpp
│   └── state_machine_example.cpp
│
├── include/
│   ├── Alice.hpp
│   ├── BB84Config.hpp
│   ├── BB84Provider.hpp
│   ├── BasisSelector.hpp
│   ├── BasisSifter.hpp
│   ├── Bob.hpp
│   ├── Cascade.hpp
│   ├── IClassicalChannel.hpp
│   ├── IQKDProvider.hpp
│   ├── IRandomGenerator.hpp
│   ├── KeyManager.hpp
│   ├── MeasurementResult.hpp
│   ├── OpenSSLRandomGenerator.hpp
│   ├── PhotonGenerator.hpp
│   ├── PrivacyAmplification.hpp
│   ├── ProviderError.hpp
│   ├── ProviderStatus.hpp
│   ├── QBER.hpp
│   ├── QuantumChannel.hpp
│   └── Sifting.hpp
│
├── src/
│   ├── Alice.cpp
│   ├── BB84Provider.cpp
│   ├── BasisSelector.cpp
│   ├── BasisSifter.cpp
│   ├── Bob.cpp
│   ├── Cascade.cpp
│   ├── ClassicalChannel.cpp
│   ├── KeyManager.cpp
│   ├── OpenSSLRandomGenerator.cpp
│   ├── PhotonGenerator.cpp
│   ├── PrivacyAmplification.cpp
│   ├── QBER.cpp
│   ├── QuantumChannel.cpp
│   └── Sifting.cpp
│
└── tests/
    ├── test_Alice.cpp
    ├── test_BasisSelector.cpp
    ├── test_BasisSifter.cpp
    ├── test_Bob.cpp
    ├── test_OpenSSLRandomGenerator.cpp
    ├── test_PhotonGenerator.cpp
    └── test_bb84_state_machine.cpp
```

---

# 21. Source Components

| Component                | Responsibility                           |
| ------------------------ | ---------------------------------------- |
| `Alice`                  | Alice-side BB84 preparation              |
| `Bob`                    | Bob-side basis selection and measurement |
| `PhotonGenerator`        | BB84 bit/state generation                |
| `BasisSelector`          | Random BB84 basis selection              |
| `OpenSSLRandomGenerator` | CSPRNG implementation                    |
| `IRandomGenerator`       | Randomness abstraction                   |
| `BasisSifter`            | Classical basis reconciliation           |
| `QuantumChannel`         | Quantum communication abstraction        |
| `IClassicalChannel`      | Classical communication abstraction      |
| `QBER`                   | Quantum bit error-rate processing        |
| `Cascade`                | Error-correction abstraction             |
| `PrivacyAmplification`   | Secret-key reduction stage               |
| `KeyManager`             | Key-management abstraction               |
| `BB84Provider`           | BB84 provider implementation             |
| `IQKDProvider`           | Public QKD provider interface            |

---

# 22. Build Requirements

The project uses:

* C++20
* CMake
* OpenSSL
* CTest
* A C++20-compatible compiler

The current development environment has also been tested with the MSYS2 UCRT64 toolchain on Windows.

---

# 23. Build

From the repository root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
```

Build the project:

```bash
cmake --build build -j
```

---

# 24. Run Tests

Run the complete CTest suite:

```bash
ctest --test-dir build --output-on-failure
```

Individual test executables are also generated under the build directory.

The current test suite covers components including:

* Alice
* BasisSelector
* BasisSifter
* Bob
* OpenSSLRandomGenerator
* PhotonGenerator
* BB84 provider state machine

---

# 25. Run Examples

After building, examples are available under the `build` directory.

### Alice example

```bash
./build/alice_example.exe
```

### Bob example

```bash
./build/bob_example.exe
```

### Basis-sifting example

```bash
./build/basis_sifter_example.exe
```

### Provider state-machine example

```bash
./build/bb84_state_machine_example.exe
```

On Linux, the generated executables may not have the `.exe` suffix.

---

# 26. Example: Randomized BB84 Sifting

The basis-sifting example uses the CSPRNG-backed implementation.

A run may produce:

```text
Alice raw bits: 00101010
Alice bases: Z X X X Z X Z Z
Bob measurements: 00101010
Bob bases: Z X Z X X Z Z X
```

Another execution can produce completely different values.

This is expected because:

```text
Alice bits      -> random
Alice bases     -> random
Bob bases       -> random
Incompatible measurements -> probabilistic
```

The sifting decision itself remains deterministic.

---

# 27. Security Design Principles

The project follows several design principles.

### Cryptographically secure randomness

Protocol randomness is provided through the `IRandomGenerator` abstraction and the `OpenSSLRandomGenerator` implementation.

### Dependency injection

Components receive required services through their constructors rather than creating hidden global dependencies.

### Separation of protocol stages

Alice, Bob, sifting, channels, QBER, error correction, privacy amplification, and key management are represented as separate components.

### Limited data exposure

Sensitive intermediate BB84 information should remain internal to the protocol pipeline and should not be unnecessarily exposed through the public provider interface.

### Testability

Interfaces and modular classes allow individual components to be tested independently.

---

# 28. Current Limitations

This repository is an active implementation and should **not yet be considered a production-ready QKD system**.

Current limitations include:

* No physical quantum-photon implementation
* No physical optical hardware interface
* No complete noisy quantum-channel model
* No complete eavesdropper/Eve model
* No production-grade authenticated classical channel
* QBER stage is still under development
* Cascade error correction is still under development
* Privacy amplification is still under development
* Key-management integration is still under development
* Full provider-to-protocol orchestration is still under development
* Security certification and formal protocol validation have not been completed

The presence of a source file or interface for a component does not necessarily mean that the component is already production-complete.

---

# 29. Development Roadmap

The intended implementation sequence is:

```text
[✓] Project architecture
        |
        v
[✓] IQKDProvider / BB84Provider foundation
        |
        v
[✓] CSPRNG abstraction
        |
        v
[✓] OpenSSL CSPRNG implementation
        |
        v
[✓] Alice raw-bit generation
        |
        v
[✓] Alice basis selection
        |
        v
[✓] BB84 state preparation
        |
        v
[✓] Bob basis selection
        |
        v
[✓] Bob measurement
        |
        v
[✓] Basis sifting
        |
        v
[ ] Quantum-channel integration
        |
        v
[ ] Classical-channel integration
        |
        v
[ ] QBER estimation
        |
        v
[ ] Cascade error correction
        |
        v
[ ] Privacy amplification
        |
        v
[ ] KeyManager integration
        |
        v
[ ] Complete BB84Provider orchestration
        |
        v
[ ] End-to-end BB84 pipeline
        |
        v
[ ] Security hardening and validation
```

---

# 30. Design Goal

The long-term goal of this project is to provide a modular BB84 implementation that can be integrated into a larger telecom/QKD security architecture through the `IQKDProvider` abstraction.

The intended final structure is:

```text
Application
     |
     v
IQKDProvider
     |
     v
BB84Provider
     |
     +----------------------------------+
     |                                  |
     v                                  v
BB84 Protocol Pipeline              Key Manager
     |
     +--> CSPRNG
     |
     +--> Alice
     |
     +--> Quantum Channel
     |
     +--> Bob
     |
     +--> Classical Channel
     |
     +--> Sifting
     |
     +--> QBER
     |
     +--> Cascade
     |
     +--> Privacy Amplification
     |
     +--> Final Key
```

The architecture is designed to allow individual components to evolve without requiring changes to the public QKD provider interface.

---

# 31. Repository Status

**Development status:** Active development

**Protocol:** BB84

**Language:** C++20

**Randomness:** OpenSSL-backed CSPRNG abstraction

**Build system:** CMake

**Testing:** CTest + component-level tests

**API architecture:** `IQKDProvider` / `BB84Provider`

**Current verified protocol path:**

```text
CSPRNG
  ↓
Alice random bits
  ↓
Alice random bases
  ↓
BB84 state preparation
  ↓
Bob random measurement bases
  ↓
Bob measurement
  ↓
Basis reconciliation
  ↓
Sifted key
```

The project is being developed incrementally, with each protocol stage independently implemented and validated before being integrated into the complete BB84 provider pipeline.
