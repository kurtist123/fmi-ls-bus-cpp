/**
 * @file hex_codec.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Hexadecimal string codec for FMI 2.0 string fallback transport
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_CODEC_HEX_CODEC_HPP
#define FMI_LS_BUS_CODEC_HEX_CODEC_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fmi_ls_bus::codec
{

/**
 * @brief Hexadecimal encoder and decoder for FMI 2.0 String fallback transport
 */
class HexCodec
{
public:
    /**
     * @brief Encodes a raw byte buffer into a hexadecimal string
     *
     * @param data Pointer to input byte buffer
     * @param size Number of bytes to encode
     * @param uppercase Whether to output uppercase hex characters (default
     * false)
     * @return Hexadecimal-encoded string
     */
    static std::string encode(
        const uint8_t* data,
        size_t size,
        bool uppercase = false
    );

    /**
     * @brief Encodes a byte vector into a hexadecimal string
     *
     * @param bytes Input byte vector to encode
     * @param uppercase Whether to output uppercase hex characters (default
     * false)
     * @return Hexadecimal-encoded string
     */
    static std::string encode(
        const std::vector<uint8_t>& bytes,
        bool uppercase = false
    );

    /**
     * @brief Decodes a hexadecimal string into an output byte vector
     *
     * @param hex_string Hexadecimal string view to decode
     * @param out_bytes Destination byte vector where decoded bytes are written
     * @return True if decoding succeeded, false if string contains invalid hex
     * characters or odd length
     */
    static bool decode(
        std::string_view hex_string,
        std::vector<uint8_t>& out_bytes
    );

    /**
     * @brief Decodes a hexadecimal string and returns optional byte vector
     *
     * @param hex_string Hexadecimal string view to decode
     * @return Optional vector with decoded bytes, or std::nullopt on invalid
     * hex input
     */
    static std::optional<std::vector<uint8_t>> decode(
        std::string_view hex_string
    );

    /**
     * @brief Checks if a string contains valid even-length hexadecimal
     * characters
     *
     * @param hex_string String view to validate
     * @return True if valid hexadecimal representation, false otherwise
     */
    static bool is_valid_hex(std::string_view hex_string) noexcept;
};

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_HEX_CODEC_HPP
