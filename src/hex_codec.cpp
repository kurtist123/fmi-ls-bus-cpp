/**
 * @file hex_codec.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Implementation of hexadecimal string encoder and decoder
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include "fmi_ls_bus/codec/hex_codec.hpp"

#include <cctype>

namespace fmi_ls_bus::codec
{

namespace
{

inline char to_hex_digit(uint8_t nibble, bool uppercase) noexcept
{
    if (nibble < 10)
    {
        return static_cast<char>('0' + nibble);
    }
    return static_cast<char>((uppercase ? 'A' : 'a') + (nibble - 10));
}

inline int from_hex_digit(char c) noexcept
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f')
    {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F')
    {
        return 10 + (c - 'A');
    }
    return -1;
}

}  // namespace

std::string HexCodec::encode(const uint8_t* data, size_t size, bool uppercase)
{
    if (data == nullptr || size == 0)
    {
        return "";
    }
    std::string result;
    result.reserve(size * 2);
    for (size_t i = 0; i < size; ++i)
    {
        const uint8_t byte = data[i];
        result.push_back(to_hex_digit((byte >> 4) & 0x0F, uppercase));
        result.push_back(to_hex_digit(byte & 0x0F, uppercase));
    }
    return result;
}

std::string HexCodec::encode(const std::vector<uint8_t>& bytes, bool uppercase)
{
    return encode(bytes.data(), bytes.size(), uppercase);
}

bool HexCodec::decode(
    std::string_view hex_string,
    std::vector<uint8_t>& out_bytes
)
{
    out_bytes.clear();
    if (hex_string.empty())
    {
        return true;
    }
    if (hex_string.size() % 2 != 0)
    {
        return false;
    }

    out_bytes.reserve(hex_string.size() / 2);
    for (size_t i = 0; i < hex_string.size(); i += 2)
    {
        const int hi = from_hex_digit(hex_string[i]);
        const int lo = from_hex_digit(hex_string[i + 1]);
        if (hi < 0 || lo < 0)
        {
            out_bytes.clear();
            return false;
        }
        out_bytes.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return true;
}

std::optional<std::vector<uint8_t>> HexCodec::decode(
    std::string_view hex_string
)
{
    std::vector<uint8_t> result;
    if (decode(hex_string, result))
    {
        return result;
    }
    return std::nullopt;
}

bool HexCodec::is_valid_hex(std::string_view hex_string) noexcept
{
    if (hex_string.size() % 2 != 0)
    {
        return false;
    }
    for (char c : hex_string)
    {
        if (from_hex_digit(c) < 0)
        {
            return false;
        }
    }
    return true;
}

}  // namespace fmi_ls_bus::codec
