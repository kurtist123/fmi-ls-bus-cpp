#ifndef FMI_LS_BUS_CODEC_HEX_CODEC_HPP
#define FMI_LS_BUS_CODEC_HEX_CODEC_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fmi_ls_bus::codec
{

class HexCodec
{
public:
    static std::string encode(
        const uint8_t* data,
        size_t size,
        bool uppercase = false
    );

    static std::string encode(
        const std::vector<uint8_t>& bytes,
        bool uppercase = false
    );

    static bool decode(
        std::string_view hex_string,
        std::vector<uint8_t>& out_bytes
    );

    static std::optional<std::vector<uint8_t>> decode(
        std::string_view hex_string
    );

    static bool is_valid_hex(std::string_view hex_string) noexcept;
};

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_HEX_CODEC_HPP
