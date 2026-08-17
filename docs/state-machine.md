# BB84 Provider State Machine

## States

```text
UNINITIALIZED
      |
      | initialize()
      v
INITIALIZED
      |
      | openConnection()
      v
READY
      |
      | getKey()
      v
KEY_GENERATION
      |
      | successful BB84 pipeline
      v
KEY_AVAILABLE
      |
      | closeConnection()
      v
INITIALIZED
```

Failure during key generation:

```text
KEY_GENERATION
      |
      | failure / QBER abort / transport failure
      v
ERROR
```

Global cleanup:

```text
UNINITIALIZED ─┐
INITIALIZED    │
READY          │ shutdown()
KEY_GENERATION ├──────────> SHUTDOWN
KEY_AVAILABLE  │
ERROR          │
              └──────────
```

## API policy

| State | Allowed operations |
|---|---|
| UNINITIALIZED | `initialize`, `getStatus`, `shutdown` |
| INITIALIZED | `openConnection`, `getStatus`, `shutdown` |
| READY | `getKey`, `getStatus`, `getQBER`, `getKeyRate`, `closeConnection`, `shutdown` |
| KEY_GENERATION | `getStatus`, `getQBER`, `getKeyRate`, `shutdown` |
| KEY_AVAILABLE | `getKey`, `getStatus`, `getQBER`, `getKeyRate`, `closeConnection`, `shutdown` |
| ERROR | `getStatus`, `shutdown` |
| SHUTDOWN | `shutdown` |

## Security decisions

1. `getKey()` returns a copy rather than exposing internal key storage.
2. Sensitive provider-owned key material is cleared during shutdown.
3. `shutdown()` is idempotent.
4. Invalid state transitions are rejected.
5. Key generation is an internal operation; applications interact through `IQKDProvider`.
6. The current random-byte generation is scaffolding only. It must be replaced by the complete BB84 pipeline before production use.
7. The authenticated classical channel is a mandatory production dependency.
8. KeyManager/KMS integration will be added before production deployment.

## Next implementation modules

The state machine is only the orchestration foundation. The next modules should be implemented independently:

- `QuantumState`
- `PhotonGenerator`
- `BasisSelector`
- `Alice`
- `QuantumChannel`
- `Bob`
- `ClassicalChannel`
- `Sifting`
- `QBER`
- `Cascade`
- `PrivacyAmplification`
- `KeyManager`
- then replace `generateKeyInternal()` with the real BB84 pipeline.
