# FMI LS-BUS Low-Cut C++ Library Architecture & FMU Integration Plan

## 1. Executive Summary

This document specifies the architectural design, wire protocol implementation, buffer management, and `fmu4cpp` integration strategy for the **`fmi-ls-bus-cpp`** library.
The library implements the official **Functional Mock-up Interface Layered Standard for Network Communication (FMI LS-BUS)** Low-Cut (Network Abstraction) protocol, specifically focusing on CAN, CAN FD, and CAN XL.

---

## 2. Low-Cut vs. High-Cut Architecture

```
+-----------------------------------------------------------------------------+
|                         FMI LS-BUS ABSTRACTION LEVELS                       |
+-----------------------------------------------------------------------------+
|                                                                             |
|  [ High-Cut: Physical Signal Abstraction ]                                  |
|  - Transports individual scalar signals (RPM, Speed, Temperature)            |
|  - FMI Variables: fmi3Float64, fmi3Int32, fmi3Boolean                        |
|  - Idealized bus: Zero-latency, infinite bandwidth, no collisions            |
|  - Limitation: Multiple messages within one do_step overwrite each other    |
|                                                                             |
|  [ Low-Cut: Network Abstraction (Implemented by fmi-ls-bus-cpp) ]            |
|  - Emulates MCAL bus controller & transceiver hardware driver               |
|  - Transports opaque binary buffers (fmi3Binary) with serialized operations  |
|  - Features: Multi-frame batching per step, arbitration lost, bus errors,    |
|    confirmations, node state transitions (Active, Passive, Bus-off)         |
|  - Clocks: Synchronized via FMI 3.0 triggered clocks (Tx_Clock, Rx_Clock)   |
|                                                                             |
+-----------------------------------------------------------------------------+
```

---

## 3. Layered Library Architecture

The library is designed with a strict three-layer decoupled architecture:

```
+-----------------------------------------------------------------------------+
| Layer 3: FMU Integration Adapter (fmi_ls_bus::adapter)                      |
| - fmu4cpp_adapter.hpp: Seamless binding to fmu4cpp::fmu_base               |
| - metadata.hpp: terminalsAndIcons.xml & fmi-ls-manifest.xml generators      |
+-----------------------------------------------------------------------------+
                                      │
                                      ▼
+-----------------------------------------------------------------------------+
| Layer 2: Channel & Buffer Queue (fmi_ls_bus::channel)                       |
| - can_channel.hpp: Bus endpoint managing TX/RX queues and step batching      |
| - frame_queue.hpp: Thread-safe / bounded ring queue with capacity management |
+-----------------------------------------------------------------------------+
                                      │
                                      ▼
+-----------------------------------------------------------------------------+
| Layer 1: Protocol & Codec (fmi_ls_bus::protocol, fmi_ls_bus::codec)         |
| - constants.hpp, types.hpp, operations.hpp: Wire structs & opcodes          |
| - serializer.hpp / deserializer.hpp: Little-endian binary byte-stream pack  |
| - hex_codec.hpp: Hex string encoder/decoder for FMI 2.0 fallback transport  |
| - endian.hpp: Platform-independent little-endian byte ordering              |
+-----------------------------------------------------------------------------+
```

### Layer 1: Protocol & Codec (`fmi_ls_bus::protocol`, `fmi_ls_bus::codec`)
- **Zero Dependencies**: Pure C++17 standard library (`<cstdint>`, `<vector>`, `<span>`, `<string>`, `<string_view>`).
- **Wire Formats**: Exact 1-byte packed alignment matching `fmi3LsBus.h` and `fmi3LsBusCan.h`.
- **Little-Endian Enforcement**: All numeric fields (e.g. `uint32_t opCode`, `uint32_t length`, `uint32_t id`, `uint16_t dataLength`) are converted to/from little-endian byte order portably regardless of host architecture.
- **Batching Support**: Serializers append multiple operations into a single continuous byte buffer. Deserializers parse consecutive operations until end-of-buffer, validating length bounds and reporting `FormatError` (`0x0001`) on corrupt bytes.
- **FMI 2.0 Fallback**: Provides `HexCodec` converting binary buffers to/from lowercase hexadecimal strings for transmission across FMI 2.0 string variables.

### Layer 2: Channel & Queue (`fmi_ls_bus::channel`)
- **`CanChannel`**: Represents a network node controller endpoint.
  - Manages outgoing TX frames and incoming RX frames.
  - Supports configurable maximum transmission buffer size (e.g. 2048 or 4096 bytes).
  - Handles notification settings (`BusNotifications = true / false`): whether `Confirm`, `ArbitrationLost`, or `BusError` are retained or silently ignored.
  - Encapsulates node state (`Active`, `Passive`, `BusOff`).
- **`FrameQueue`**: FIFO queue holding operations awaiting transmission or dispatch to application code.

### Layer 3: FMU Integration Adapter (`fmi_ls_bus::adapter`)
- **`Fmu4cppCanAdapter`**:
  - Connects `CanChannel` directly to `fmu4cpp::fmu_base`.
  - Automatically registers the four required FMI 3.0 variables:
    1. `Tx_Data` (`fmi3Binary`, output, causality discrete)
    2. `Tx_Clock` (`fmi3Clock`, output, interval triggered)
    3. `Rx_Data` (`fmi3Binary`, input, causality discrete)
    4. `Rx_Clock` (`fmi3Clock`, input, interval triggered)
  - Hooks into FMI 3.0 Co-Simulation lifecycle:
    - Step processing (`do_step`): Serializes queued TX frames into `Tx_Data`. If frames are pending, triggers `Tx_Clock` via `fmu_base::notify_output_event(tx_clock_vr)`.
    - Event Mode (`on_enter_event_mode`, `update_discrete_states`): Inspects `Rx_Clock`. If active, deserializes incoming `Rx_Data` into `CanChannel` RX queue.
- **`MetadataGenerator`**:
  - Programmatically generates valid `terminalsAndIcons.xml` declaring `terminalKind="org.fmi-ls-bus.network-terminal"` with matching rule `"org.fmi-ls-bus.transceiver"`.
  - Generates `/extra/org.fmi-standard.fmi-ls-bus/fmi-ls-manifest.xml` adhering to `fmi3LayeredStandardBusManifest.xsd`.

---

## 4. FMI 3.0 Lifecycle & Event Mode Execution Sequence

```
 Sending FMU (Node A)             FMI 3.0 Co-Sim Master            Receiving FMU (Node B)
        │                                   │                                  │
        │── fmi3DoStep(t, dt) ─────────────>│<──────── fmi3DoStep(t, dt) ──────│
        │   [Queues CAN messages]           │          [Internal simulation]   │
        │   [Tx_Data packed]                │                                  │
        │   [Tx_Clock activated]            │                                  │
        │<── eventHandlingNeeded=true ──────│── eventHandlingNeeded=false ────>│
        │                                   │                                  │
        │── fmi3GetClock(Tx_Clock=true) ───>│                                  │
        │── fmi3GetBinary(Tx_Data) ────────>│                                  │
        │                                   │── fmi3SetBinary(Rx_Data) ───────>│
        │                                   │── fmi3SetClock(Rx_Clock=true) ──>│
        │                                   │                                  │
        │<── fmi3EnterEventMode() ──────────│── fmi3EnterEventMode() ─────────>│
        │<── fmi3UpdateDiscreteStates() ────│── fmi3UpdateDiscreteStates() ───>│
        │    [Tx buffer cleared]            │   [Rx buffer unpacked]           │
        │    [Clocks reset]                 │   [Messages processed]           │
        │                                   │   [Clocks reset]                 │
        │<── fmi3EnterStepMode() ───────────│── fmi3EnterStepMode() ──────────>│
        │                                   │                                  │
```

---

## 5. FMI 2.0 Backward Compatibility Strategy

Since FMI 2.0 does not natively support `fmi3Binary`, `fmi3Clock`, or `EventMode` in Co-Simulation, the library provides a standardized emulation layer:

1. **Transport via `fmi2String`**:
   The packed binary byte buffer is encoded into a hexadecimal string using `HexCodec::encode(buffer)` (2 hex characters per byte, uppercase or lowercase).
2. **Handshake via Discrete Variables**:
   - `Tx_Data_Hex` (String, output)
   - `Tx_Active` (Boolean, output) or `Tx_Seq` (Integer, output incrementing counter)
   - `Rx_Data_Hex` (String, input)
   - `Rx_Active` (Boolean, input) or `Rx_Seq` (Integer, input)
3. **Step Boundary Processing**:
   At each `fmi2DoStep`, if `Rx_Active` is true (or `Rx_Seq` changed), the receiving FMU decodes `Rx_Data_Hex` with `HexCodec::decode(...)` and dispatches operations to its `CanChannel`.

---

## 6. Implementation Milestones

- **Milestone 1**: Documentation, specification archiving, and formatting configuration (Completed).
- **Milestone 2**: Core C++ library implementation (`protocol`, `codec`, `channel`, `adapter`).
- **Milestone 3**: CMake build configuration with Google Test `FetchContent`, comprehensive unit tests, and GitHub Actions CI.
- **Milestone 4**: Complete verification, clang-format validation, test suite execution, and audit readiness.
