#ifndef FMI_LS_BUS_CODEC_DESERIALIZER_HPP
#define FMI_LS_BUS_CODEC_DESERIALIZER_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "fmi_ls_bus/protocol/operations.hpp"

namespace fmi_ls_bus::codec
{

struct DeserializationResult
{
    bool success{false};
    size_t bytes_consumed{0};
    protocol::BusOperation operation{protocol::Wakeup{}};
    std::string error_message;
};

class Deserializer
{
public:
    static DeserializationResult deserialize_next(
        const uint8_t* data,
        size_t size
    );

    static std::vector<protocol::BusOperation> deserialize_all(
        const uint8_t* data,
        size_t size
    );

    static std::vector<protocol::BusOperation> deserialize_all(
        const std::vector<uint8_t>& buffer
    );
};

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_DESERIALIZER_HPP
