/**
 * @file types.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Protocol enumeration types, error codes, and flags for FMI LS-BUS
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_PROTOCOL_TYPES_HPP
#define FMI_LS_BUS_PROTOCOL_TYPES_HPP

#include <cstddef>
#include <cstdint>

#include "fmi_ls_bus/protocol/constants.hpp"

namespace fmi_ls_bus::protocol
{

#pragma pack(push, 1)

/**
 * @brief Common 8-byte operation header present at start of every wire frame
 */
struct WireHeader
{
    uint32_t op_code;
    uint32_t length;
};

static_assert(sizeof(WireHeader) == 8, "WireHeader must be 8 bytes");

/**
 * @brief Fixed wire header layout for FormatError operations
 */
struct WireFormatErrorHeader
{
    WireHeader header;
    uint16_t data_length;
};

static_assert(
    sizeof(WireFormatErrorHeader) == 10,
    "WireFormatErrorHeader must be 10 bytes"
);

/**
 * @brief Fixed wire header layout for classic CAN transmit operations
 */
struct WireCanTransmitHeader
{
    WireHeader header;
    uint32_t id;
    uint8_t ide;
    uint8_t rtr;
    uint16_t data_length;
};

static_assert(
    sizeof(WireCanTransmitHeader) == 16,
    "WireCanTransmitHeader must be 16 bytes"
);

/**
 * @brief Fixed wire header layout for CAN-FD transmit operations
 */
struct WireCanFdTransmitHeader
{
    WireHeader header;
    uint32_t id;
    uint8_t ide;
    uint8_t brs;
    uint8_t esi;
    uint16_t data_length;
};

static_assert(
    sizeof(WireCanFdTransmitHeader) == 17,
    "WireCanFdTransmitHeader must be 17 bytes"
);

/**
 * @brief Fixed wire header layout for CAN-XL transmit operations
 */
struct WireCanXlTransmitHeader
{
    WireHeader header;
    uint32_t id;
    uint8_t ide;
    uint8_t sec;
    uint8_t sdt;
    uint8_t vcid;
    uint32_t af;
    uint16_t data_length;
};

static_assert(
    sizeof(WireCanXlTransmitHeader) == 22,
    "WireCanXlTransmitHeader must be 22 bytes"
);

/**
 * @brief Wire layout for transmission confirmation operations
 */
struct WireConfirm
{
    WireHeader header;
    uint32_t id;
};

static_assert(sizeof(WireConfirm) == 12, "WireConfirm must be 12 bytes");

/**
 * @brief Wire layout for arbitration loss notification operations
 */
struct WireArbitrationLost
{
    WireHeader header;
    uint32_t id;
};

static_assert(
    sizeof(WireArbitrationLost) == 12,
    "WireArbitrationLost must be 12 bytes"
);

/**
 * @brief Wire layout for bus error notification operations
 */
struct WireBusError
{
    WireHeader header;
    uint32_t id;
    uint8_t error_code;
    uint8_t error_flag;
    uint8_t is_sender;
};

static_assert(sizeof(WireBusError) == 15, "WireBusError must be 15 bytes");

/**
 * @brief Wire layout for node status notification operations
 */
struct WireStatus
{
    WireHeader header;
    uint8_t status;
};

static_assert(sizeof(WireStatus) == 9, "WireStatus must be 9 bytes");

/**
 * @brief Wire layout for bus wakeup operations
 */
struct WireWakeup
{
    WireHeader header;
};

static_assert(sizeof(WireWakeup) == 8, "WireWakeup must be 8 bytes");

/**
 * @brief Wire layout for baudrate configuration parameter operations
 */
struct WireConfigurationBaudrate
{
    WireHeader header;
    uint8_t parameter_type;
    uint32_t baudrate;
};

static_assert(
    sizeof(WireConfigurationBaudrate) == 13,
    "WireConfigurationBaudrate must be 13 bytes"
);

/**
 * @brief Wire layout for arbitration lost behavior configuration operations
 */
struct WireConfigurationArbitration
{
    WireHeader header;
    uint8_t parameter_type;
    uint8_t arbitration_lost_behavior;
};

static_assert(
    sizeof(WireConfigurationArbitration) == 10,
    "WireConfigurationArbitration must be 10 bytes"
);

#pragma pack(pop)

}  // namespace fmi_ls_bus::protocol

#endif  // FMI_LS_BUS_PROTOCOL_TYPES_HPP
