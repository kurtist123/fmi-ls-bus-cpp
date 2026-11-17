/**
 * @file deserializer.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Binary deserializer for FMI LS-BUS low-cut operations
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_CODEC_DESERIALIZER_HPP
#define FMI_LS_BUS_CODEC_DESERIALIZER_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "fmi_ls_bus/protocol/operations.hpp"

namespace fmi_ls_bus::codec
{

/**
 * @brief Result structure returned when attempting to deserialize a wire
 * operation
 */
struct DeserializationResult
{
    bool success{false};
    size_t bytes_consumed{0};
    protocol::BusOperation operation{protocol::Wakeup{}};
    std::string error_message;
};

/**
 * @brief Binary deserializer decoding wire byte streams into FMI LS-BUS
 * operations
 */
class Deserializer
{
public:
    /**
     * @brief Deserializes the next operation from a byte buffer
     *
     * @param data Pointer to raw input byte buffer
     * @param size Number of available bytes in data buffer
     * @return DeserializationResult detailing success, bytes consumed, and
     * operation or error
     */
    static DeserializationResult deserialize_next(
        const uint8_t* data,
        size_t size
    );

    /**
     * @brief Deserializes all operations sequentially from a raw byte buffer
     *
     * @param data Pointer to raw input byte buffer
     * @param size Number of available bytes in data buffer
     * @return Vector of successfully decoded BusOperation variants
     */
    static std::vector<protocol::BusOperation> deserialize_all(
        const uint8_t* data,
        size_t size
    );

    /**
     * @brief Deserializes all operations sequentially from a vector buffer
     *
     * @param buffer Byte vector containing serialized wire stream
     * @return Vector of successfully decoded BusOperation variants
     */
    static std::vector<protocol::BusOperation> deserialize_all(
        const std::vector<uint8_t>& buffer
    );
};

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_DESERIALIZER_HPP
