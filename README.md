# bb84-engine

Initial implementation of the IQKDProvider/BB84Provider architecture.

## Current milestone

This milestone implements the **provider state machine and public API contract**.

It does **not** yet implement the real BB84 cryptographic pipeline.

The temporary `generateKeyInternal()` exists only to validate lifecycle/orchestration behavior. It must not be used as a production QKD key generator.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

## Architecture

```text
Application
     |
     v
IQKDProvider
     |
     v
BB84Provider
     |
     +--> KeyManager/KMS
     |
     +--> BB84 pipeline
            |
            +--> PhotonGenerator
            +--> BasisSelector
            +--> Alice
            +--> QuantumChannel
            +--> Bob
            +--> ClassicalChannel
            +--> Sifting
            +--> QBER
            +--> Cascade
            +--> PrivacyAmplification
            +--> KeyManager
```

## Important

No intermediate BB84 data, raw key material, or partial key is exposed through `IQKDProvider`.
