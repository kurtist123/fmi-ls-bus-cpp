#ifndef FMI_LS_BUS_CODEC_SERIALIZER_HPP
#define FMI_LS_BUS_CODEC_SERIALIZER_HPP

#include <cstdint>
#include <vector>

#include "fmi_ls_bus/protocol/operations.hpp"

namespace fmi_ls_bus::codec
{

class Serializer
{
public:
    static void serialize(
        const protocol::FormatError& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::CanTransmit& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::CanFdTransmit& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::CanXlTransmit& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::Confirm& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::ArbitrationLost& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::BusError& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::Configuration& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::Status& op,
        std::vector<uint8_t>& out
    );
    static void serialize(
        const protocol::Wakeup& op,
        std::vector<uint8_t>& out
    );

    static void serialize(
        const protocol::BusOperation& op,
        std::vector<uint8_t>& out
    );
    static std::vector<uint8_t> serialize(const protocol::BusOperation& op);

    static void serialize_batch(
        const std::vector<protocol::BusOperation>& ops,
        std::vector<uint8_t>& out
    );
    static std::vector<uint8_t> serialize_batch(
        const std::vector<protocol::BusOperation>& ops
    );
};

}  // namespace fmi_ls_bus::codec

#endif  // FMI_LS_BUS_CODEC_SERIALIZER_HPP
