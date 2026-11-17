# fmi-ls-bus-cpp

A standalone C++17 library implementing the **Functional Mock-up Interface Layered Standard for Network Communication (FMI LS-BUS)** Low-Cut (Network Abstraction) protocol.

Designed for virtual bus simulation, multi-frame batching, and integration with FMUs built using [fmu4cpp](https://github.com/Ecos-platform/fmu4cpp).

## Documentation & Specification

- [Architecture & Implementation Plan](docs/IMPLEMENTATION_PLAN.md)
- [Repository Naming Proposal & Evaluation](docs/REPOSITORY_NAMING_PROPOSAL.md)
- [Official FMI LS-BUS Specifications](docs/spec/README.md)

## Features

- **FMI LS-BUS Low-Cut Protocol**: Supports CAN 2.0, CAN FD, and CAN XL frames, bus error notifications, and arbitration lost handling.
- **High-Performance Binary Wire Codecs**: Little-endian binary serialization/deserialization for FMI 3.0 `fmi3Binary` variables, plus hex fallback string encoding for FMI 2.0.
- **Decoupled Architecture**: Zero-dependency standalone core (C++17) with optional adapters for `fmu4cpp`.
- **Thread-Safe & Bounded Channel Queues**: Producer-consumer frame queues with drop-oldest, drop-newest, or reject policies.
- **Automated Metadata Generation**: Generates `terminalsAndIcons.xml` and `fmi-ls-manifest.xml` for FMI 3.0 terminals and layered standard compliance.
- **Cross-Platform CMake Build**: Modern CMake with Google Test integration via `FetchContent`.
