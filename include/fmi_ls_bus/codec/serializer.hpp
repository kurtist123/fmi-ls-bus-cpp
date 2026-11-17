/**
 * @file serializer.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Binary serializer for FMI LS-BUS low-cut operations
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_CODEC_SERIALIZER_HPP
#define FMI_LS_BUS_CODEC_SERIALIZER_HPP

#include <cstdint>
#include <vector>

#include "fmi_ls_bus/protocol/operations.hpp"

namespace fmi_ls_bus::codec
{

/**
 * @brief Binary serializer encoding FMI LS-BUS low-cut operations into wire
 * format
 */
class Serializer
{
public:
    /**
     * @brief Serializes a FormatError operation into a byte buffer
     *
     * @param op FormatError operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::FormatError& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a classic CAN transmit operation into a byte buffer
     *
     * @param op CanTransmit operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::CanTransmit& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a CAN-FD transmit operation into a byte buffer
     *
     * @param op CanFdTransmit operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::CanFdTransmit& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a CAN-XL transmit operation into a byte buffer
     *
     * @param op CanXlTransmit operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::CanXlTransmit& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a Confirm operation into a byte buffer
     *
     * @param op Confirm operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::Confirm& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes an ArbitrationLost operation into a byte buffer
     *
     * @param op ArbitrationLost operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::ArbitrationLost& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a BusError operation into a byte buffer
     *
     * @param op BusError operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::BusError& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a Configuration operation into a byte buffer
     *
     * @param op Configuration operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::Configuration& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a Status operation into a byte buffer
     *
     * @param op Status operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::Status& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a Wakeup operation into a byte buffer
     *
     * @param op Wakeup operation to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::Wakeup& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes any BusOperation variant alternative into a byte buffer
     *
     * @param op BusOperation variant to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize(
        const protocol::BusOperation& op,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes any BusOperation variant alternative and returns the
     * encoded vector
     *
     * @param op BusOperation variant to serialize
     * @return Vector containing encoded wire bytes
     */
    static std::vector<uint8_t> serialize(const protocol::BusOperation& op);

    /**
     * @brief Serializes a batch of BusOperation variants into a byte buffer
     *
     * @param ops Collection of BusOperation variants to serialize
     * @param out Destination byte buffer where encoded bytes are appended
     */
    static void serialize_batch(
        const std::vector<protocol::BusOperation>& ops,
        std::vector<uint8_t>& out
    );

    /**
     * @brief Serializes a batch of BusOperation variants and returns the
     * encoded vector
     *
     * @param ops Collection of BusOperation variants to serialize
     * @return Vector containing concatenated wire bytes for all operations
     */
    static std::vector<uint8_t> serialize_batch(
        const std::vector<protocol::BusOperation>& ops
    );
};

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_SERIALIZER_HPP
