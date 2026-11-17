# Repository Naming Proposal: FMI LS-BUS C++ Implementation

## Executive Summary

This document evaluates candidate names for the standalone C++ library implementing the **Functional Mock-up Interface Layered Standard for Network Communication (FMI LS-BUS)** Low-Cut (Network Abstraction) protocol.

The recommended primary name for this repository is **`fmi-ls-bus-cpp`**.

---

## 1. Candidate Names Evaluation

| Candidate Name | Pros | Cons | Recommendation Score (1-10) |
|---|---|---|:---:|
| **`fmi-ls-bus-cpp`** | • Directly mirrors official Modelica standard name (`fmi-ls-bus`).<br>• Clear language qualifier (`-cpp`), conforming to modern C++ library naming conventions (e.g. `fmu4cpp`, `json-cpp`, `grpc-cpp`).<br>• Precise scope: covers the FMI LS-BUS standard, not limited to FMUs or importers.<br>• Clean namespace mapping: `fmi_ls_bus` in C++ and `fmi_ls_bus::fmi_ls_bus` in CMake. | Slightly longer than acronyms. | **9.8 / 10** (Primary Choice) |
| **`fmu-ls-bus`** | • Compact and easy to remember.<br>• Connects intuitively with FMU developers. | Conflates the Layered Standard (which is an FMI specification, not an FMU instance) with an FMU.<br>Does not indicate the implementation language. | **7.5 / 10** |
| **`libfmi3-ls-bus`** | • Traditional Unix library naming (`lib*`).<br>• Highlights FMI 3.0 focus. | Overly tied to FMI 3.0 naming, whereas the layered standard itself is `fmi-ls-bus` and may support FMI 2.0 fallback transport.<br>Unix prefix `lib` is redundant on GitHub repository names. | **7.0 / 10** |
| **`fmi-bus-cpp`** | • Short and concise. | Omits `ls-` (Layered Standard), creating ambiguity with non-standard bus interfaces or legacy proprietary tools. | **6.5 / 10** |
| **`fmu4cpp-ls-bus`** | • Highlights deep integration with `fmu4cpp`. | Incorrectly implies tight coupling. The core codec, frame queue, and protocol types are zero-dependency standard C++17 that can be used standalone in importers, bus simulators, or other FMU frameworks. | **6.0 / 10** |

---

## 2. Recommended Choice: `fmi-ls-bus-cpp`

### 2.1 Alignment with FMI Ecosystem
1. **Modelica Association Convention**:
   The Modelica Association maintains official repositories named `fmi-standard`, `fmi-ls-bus`, `fmi-ls-bus-guides`, and `fmi-ls-bus-examples`. Using `fmi-ls-bus-cpp` establishes direct semantic alignment with official specifications.
2. **Package & Library Conventions**:
   - CMake target: `fmi_ls_bus_cpp` (alias `fmi_ls_bus::fmi_ls_bus`)
   - C++ namespace: `fmi_ls_bus` (with sub-namespaces `protocol`, `codec`, `channel`, `adapter`)
   - Header directory: `include/fmi_ls_bus/`
   - vcpkg / Conan package: `fmi-ls-bus-cpp`

### 2.2 Decoupled Usability
While the library provides first-class adapters for `fmu4cpp`, it is architected as an independent standalone C++ library. Importers, bus simulation masters, and testing harnesses can consume `fmi-ls-bus-cpp` without pulling in `fmu4cpp` or any FMU runtime.

---

## 3. Alternative Choices & Aliases

1. **GitHub Repository**: `kurtist123/fmi-ls-bus-cpp` (or organization repository).
2. **Git Submodule / Directory Name**: `fmi-ls-bus-cpp` or `fmi_ls_bus`.
3. **CMake Package Export**: `fmi_ls_busConfig.cmake` enabling `find_package(fmi_ls_bus REQUIRED)`.
