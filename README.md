# fmi-ls-bus-cpp

[![CI](https://github.com/kurtist123/fmi-ls-bus-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/kurtist123/fmi-ls-bus-cpp/actions/workflows/ci.yml)

A standalone C++17 library implementing the **Functional Mock-up Interface Layered Standard for Network Communication (FMI LS-BUS)** Low-Cut (Network Abstraction) protocol.

Designed for virtual bus simulation, multi-frame batching, and integration with FMUs built using [fmu4cpp](https://github.com/Ecos-platform/fmu4cpp).

## Documentation & Specification

- [Architecture & Implementation Plan](docs/IMPLEMENTATION_PLAN.md)
- [Repository Naming Proposal & Evaluation](docs/REPOSITORY_NAMING_PROPOSAL.md)
- [Official FMI LS-BUS Specifications](docs/spec/README.md)

## Architecture Overview

The library features a strictly decoupled three-layer architecture:

1. **Protocol & Codec Layer (`fmi_ls_bus::protocol`, `fmi_ls_bus::codec`)**
   - Protocol types: CAN 2.0, CAN FD, CAN XL, Error notifications, Arbitration lost, Configuration, Status, Wakeup.
   - High-performance zero-copy / little-endian binary serialization and deserialization conforming to FMI 3.0 `fmi3Binary` variables.
   - Hex-string fallback encoder/decoder for FMI 2.0 string variables.
2. **Channel & Buffer Layer (`fmi_ls_bus::channel`)**
   - `FrameQueue`: Thread-safe, bounded FIFO queue with configurable overflow policies (drop-oldest, drop-newest, reject).
   - `CanChannel`: End-point managing TX and RX frame batching per simulation step, notification filtering, and bus-off error states.
3. **FMU Integration Adapter Layer (`fmi_ls_bus::adapter`)**
   - `Fmu4cppCanAdapter`: Connects CAN channel TX/RX queues to `fmu4cpp::fmu_base` binary variables and triggered clocks (`Tx_Clock`, `Rx_Clock`).
   - `MetadataGenerator`: Generates `terminalsAndIcons.xml` and `fmi-ls-manifest.xml` for FMI 3.0 layered standard compliance.

## Quick Start & Building

### Requirements
- C++17 compliant compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.20+
- `clang-format` (for code formatting verification)

### Build & Run Tests

```bash
# Configure
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DFMI_LS_BUS_BUILD_TESTS=ON

# Build
cmake --build build --config Release --parallel

# Run Unit Tests
ctest --test-dir build --output-on-failure
```

Google Test is fetched automatically via `FetchContent`.

### Code Formatting

The codebase enforces strict clang-format rules:

```bash
# Check formatting
find include src tests -name '*.hpp' -o -name '*.cpp' | xargs clang-format --dry-run --Werror

# Auto-format
find include src tests -name '*.hpp' -o -name '*.cpp' | xargs clang-format -i
```
