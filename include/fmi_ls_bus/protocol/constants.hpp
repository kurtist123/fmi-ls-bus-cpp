/**
 * @file constants.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Protocol constants, magic bytes, opcodes, and size limits for FMI
 * LS-BUS
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_PROTOCOL_CONSTANTS_HPP
#define FMI_LS_BUS_PROTOCOL_CONSTANTS_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace fmi_ls_bus::protocol
{

// Boolean constants for wire serialization
inline constexpr uint8_t WIRE_FALSE = 0x00;
inline constexpr uint8_t WIRE_TRUE = 0x01;

// Header size
inline constexpr size_t OPERATION_HEADER_SIZE = 8;

// Fixed header sizes for operations (excluding variable payload data)
inline constexpr size_t FORMAT_ERROR_FIXED_SIZE = 10;
inline constexpr size_t CAN_TRANSMIT_FIXED_SIZE = 16;
inline constexpr size_t CAN_FD_TRANSMIT_FIXED_SIZE = 17;
inline constexpr size_t CAN_XL_TRANSMIT_FIXED_SIZE = 22;
inline constexpr size_t CONFIRM_SIZE = 12;
inline constexpr size_t ARBITRATION_LOST_SIZE = 12;
inline constexpr size_t BUS_ERROR_SIZE = 15;
inline constexpr size_t STATUS_SIZE = 9;
inline constexpr size_t WAKEUP_SIZE = 8;
inline constexpr size_t CONFIGURATION_BAUDRATE_SIZE = 13;
inline constexpr size_t CONFIGURATION_ARB_SIZE = 10;

// Maximum payload lengths
inline constexpr size_t CAN_MAX_PAYLOAD = 8;
inline constexpr size_t CAN_FD_MAX_PAYLOAD = 64;
inline constexpr size_t CAN_XL_MAX_PAYLOAD = 2'048;

// Operation Codes
enum class OpCode : uint32_t
{
    FormatError = 0x0001,
    CanTransmit = 0x0010,
    CanFdTransmit = 0x0011,
    CanXlTransmit = 0x0012,
    Confirm = 0x0020,
    ArbitrationLost = 0x0030,
    BusError = 0x0031,
    Configuration = 0x0040,
    Status = 0x0041,
    Wakeup = 0x0042
};

// CAN Bus Error Codes
enum class BusErrorCode : uint8_t
{
    BitError = 0x01,
    BitStuffingError = 0x02,
    FormError = 0x03,
    CrcError = 0x04,
    AckError = 0x05,
    BrokenErrorFrame = 0x06
};

// Error Flags
enum class ErrorFlag : uint8_t
{
    Primary = 0x01,
    Secondary = 0x02
};

// Node Status
enum class NodeStatus : uint8_t
{
    ErrorActive = 0x01,
    ErrorPassive = 0x02,
    BusOff = 0x03
};

// Configuration Parameter Type
enum class ConfigParamType : uint8_t
{
    CanBaudrate = 0x01,
    CanFdBaudrate = 0x02,
    CanXlBaudrate = 0x03,
    ArbitrationLostBehavior = 0x04
};

// Arbitration Lost Behavior
enum class ArbitrationLostBehavior : uint8_t
{
    BufferAndRetransmit = 0x01,
    DiscardAndNotify = 0x02
};

// Standard MIME types
inline constexpr std::string_view CAN_MIME_TYPE =
    "application/org.fmi-standard.fmi-ls-bus.can; version=\"1.0.0\"";
inline constexpr std::string_view LIN_MIME_TYPE =
    "application/org.fmi-standard.fmi-ls-bus.lin; version=\"1.0.0-rc.1\"";
inline constexpr std::string_view FLEXRAY_MIME_TYPE =
    "application/org.fmi-standard.fmi-ls-bus.flexray; version=\"1.0.0-rc.1\"";
inline constexpr std::string_view ETHERNET_MIME_TYPE =
    "application/org.fmi-standard.fmi-ls-bus.ethernet; version=\"1.0.0-rc.1\"";

// FMI Terminal Standard Names
inline constexpr std::string_view TERMINAL_KIND_NETWORK =
    "org.fmi-ls-bus.network-terminal";
inline constexpr std::string_view TERMINAL_MATCHING_RULE_TRANSCEIVER =
    "org.fmi-ls-bus.transceiver";
inline constexpr std::string_view TERMINAL_MEMBER_TX_DATA = "Tx_Data";
inline constexpr std::string_view TERMINAL_MEMBER_TX_CLOCK = "Tx_Clock";
inline constexpr std::string_view TERMINAL_MEMBER_RX_DATA = "Rx_Data";
inline constexpr std::string_view TERMINAL_MEMBER_RX_CLOCK = "Rx_Clock";
inline constexpr std::string_view TERMINAL_KIND_CONFIGURATION =
    "org.fmi-ls-bus.network-terminal.configuration";
inline constexpr std::string_view TERMINAL_MATCHING_RULE_BUS = "bus";
inline constexpr std::string_view TERMINAL_MEMBER_BUS_NOTIFICATIONS =
    "BusNotifications";

}  // namespace fmi_ls_bus::protocol

#endif  // FMI_LS_BUS_PROTOCOL_CONSTANTS_HPP
