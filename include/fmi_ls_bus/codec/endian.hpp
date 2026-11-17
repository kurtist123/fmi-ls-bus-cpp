/**
 * @file endian.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Little-endian encoding and decoding utilities for wire serialization
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_CODEC_ENDIAN_HPP
#define FMI_LS_BUS_CODEC_ENDIAN_HPP

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace fmi_ls_bus::codec
{

inline uint16_t read_le_u16(const uint8_t* src) noexcept
{
    return static_cast<uint16_t>(src[0]) | (static_cast<uint16_t>(src[1]) << 8);
}

inline uint32_t read_le_u32(const uint8_t* src) noexcept
{
    return static_cast<uint32_t>(src[0]) |
           (static_cast<uint32_t>(src[1]) << 8) |
           (static_cast<uint32_t>(src[2]) << 16) |
           (static_cast<uint32_t>(src[3]) << 24);
}

inline uint64_t read_le_u64(const uint8_t* src) noexcept
{
    uint64_t val = 0;
    for (size_t i = 0; i < 8; ++i)
    {
        val |= (static_cast<uint64_t>(src[i]) << (i * 8));
    }
    return val;
}

inline void write_le_u16(uint8_t* dst, uint16_t val) noexcept
{
    dst[0] = static_cast<uint8_t>(val & 0xFF);
    dst[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
}

inline void write_le_u32(uint8_t* dst, uint32_t val) noexcept
{
    dst[0] = static_cast<uint8_t>(val & 0xFF);
    dst[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
    dst[2] = static_cast<uint8_t>((val >> 16) & 0xFF);
    dst[3] = static_cast<uint8_t>((val >> 24) & 0xFF);
}

inline void write_le_u64(uint8_t* dst, uint64_t val) noexcept
{
    for (size_t i = 0; i < 8; ++i)
    {
        dst[i] = static_cast<uint8_t>((val >> (i * 8)) & 0xFF);
    }
}

template <typename T>
inline T read_le(const uint8_t* src) noexcept
{
    static_assert(std::is_integral_v<T>, "Integral type required");
    if constexpr (sizeof(T) == 1)
    {
        return static_cast<T>(*src);
    }
    else if constexpr (sizeof(T) == 2)
    {
        return static_cast<T>(read_le_u16(src));
    }
    else if constexpr (sizeof(T) == 4)
    {
        return static_cast<T>(read_le_u32(src));
    }
    else if constexpr (sizeof(T) == 8)
    {
        return static_cast<T>(read_le_u64(src));
    }
}

template <typename T>
inline void write_le(uint8_t* dst, T val) noexcept
{
    static_assert(std::is_integral_v<T>, "Integral type required");
    if constexpr (sizeof(T) == 1)
    {
        *dst = static_cast<uint8_t>(val);
    }
    else if constexpr (sizeof(T) == 2)
    {
        write_le_u16(dst, static_cast<uint16_t>(val));
    }
    else if constexpr (sizeof(T) == 4)
    {
        write_le_u32(dst, static_cast<uint32_t>(val));
    }
    else if constexpr (sizeof(T) == 8)
    {
        write_le_u64(dst, static_cast<uint64_t>(val));
    }
}

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_ENDIAN_HPP
