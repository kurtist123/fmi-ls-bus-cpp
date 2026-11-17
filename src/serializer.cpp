/**
 * @file serializer.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Implementation of binary wire serializer for FMI LS-BUS
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include "fmi_ls_bus/codec/serializer.hpp"

#include <cstring>

#include "fmi_ls_bus/codec/endian.hpp"
#include "fmi_ls_bus/protocol/constants.hpp"

namespace fmi_ls_bus::codec
{

void Serializer::serialize(
    const protocol::FormatError& op,
    std::vector<uint8_t>& out
)
{
    const uint32_t total_len = static_cast<uint32_t>(
        protocol::FORMAT_ERROR_FIXED_SIZE + op.raw_data.size()
    );
    const size_t offset = out.size();
    out.resize(offset + total_len);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::FormatError));
    write_le_u32(ptr + 4, total_len);
    write_le_u16(ptr + 8, static_cast<uint16_t>(op.raw_data.size()));
    if (!op.raw_data.empty())
    {
        std::memcpy(ptr + 10, op.raw_data.data(), op.raw_data.size());
    }
}

void Serializer::serialize(
    const protocol::CanTransmit& op,
    std::vector<uint8_t>& out
)
{
    const uint16_t wire_data_len = op.remote_frame ?
                                       op.requested_rtr_length :
                                       static_cast<uint16_t>(op.data.size());
    const size_t payload_bytes = op.remote_frame ? 0 : op.data.size();
    const uint32_t total_len = static_cast<uint32_t>(
        protocol::CAN_TRANSMIT_FIXED_SIZE + payload_bytes
    );

    const size_t offset = out.size();
    out.resize(offset + total_len);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::CanTransmit));
    write_le_u32(ptr + 4, total_len);
    write_le_u32(ptr + 8, op.id);
    ptr[12] = op.extended_id ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    ptr[13] = op.remote_frame ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    write_le_u16(ptr + 14, wire_data_len);
    if (payload_bytes > 0)
    {
        std::memcpy(ptr + 16, op.data.data(), payload_bytes);
    }
}

void Serializer::serialize(
    const protocol::CanFdTransmit& op,
    std::vector<uint8_t>& out
)
{
    const uint32_t total_len = static_cast<uint32_t>(
        protocol::CAN_FD_TRANSMIT_FIXED_SIZE + op.data.size()
    );
    const size_t offset = out.size();
    out.resize(offset + total_len);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::CanFdTransmit));
    write_le_u32(ptr + 4, total_len);
    write_le_u32(ptr + 8, op.id);
    ptr[12] = op.extended_id ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    ptr[13] = op.bit_rate_switch ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    ptr[14] =
        op.error_state_indicator ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    write_le_u16(ptr + 15, static_cast<uint16_t>(op.data.size()));
    if (!op.data.empty())
    {
        std::memcpy(ptr + 17, op.data.data(), op.data.size());
    }
}

void Serializer::serialize(
    const protocol::CanXlTransmit& op,
    std::vector<uint8_t>& out
)
{
    const uint32_t total_len = static_cast<uint32_t>(
        protocol::CAN_XL_TRANSMIT_FIXED_SIZE + op.data.size()
    );
    const size_t offset = out.size();
    out.resize(offset + total_len);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::CanXlTransmit));
    write_le_u32(ptr + 4, total_len);
    write_le_u32(ptr + 8, op.id);
    ptr[12] = op.extended_id ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    ptr[13] =
        op.simple_extended_content ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
    ptr[14] = op.sdu_type;
    ptr[15] = op.vcid;
    write_le_u32(ptr + 16, op.acceptance_field);
    write_le_u16(ptr + 20, static_cast<uint16_t>(op.data.size()));
    if (!op.data.empty())
    {
        std::memcpy(ptr + 22, op.data.data(), op.data.size());
    }
}

void Serializer::serialize(
    const protocol::Confirm& op,
    std::vector<uint8_t>& out
)
{
    const size_t offset = out.size();
    out.resize(offset + protocol::CONFIRM_SIZE);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::Confirm));
    write_le_u32(ptr + 4, static_cast<uint32_t>(protocol::CONFIRM_SIZE));
    write_le_u32(ptr + 8, op.id);
}

void Serializer::serialize(
    const protocol::ArbitrationLost& op,
    std::vector<uint8_t>& out
)
{
    const size_t offset = out.size();
    out.resize(offset + protocol::ARBITRATION_LOST_SIZE);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::ArbitrationLost));
    write_le_u32(
        ptr + 4,
        static_cast<uint32_t>(protocol::ARBITRATION_LOST_SIZE)
    );
    write_le_u32(ptr + 8, op.id);
}

void Serializer::serialize(
    const protocol::BusError& op,
    std::vector<uint8_t>& out
)
{
    const size_t offset = out.size();
    out.resize(offset + protocol::BUS_ERROR_SIZE);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::BusError));
    write_le_u32(ptr + 4, static_cast<uint32_t>(protocol::BUS_ERROR_SIZE));
    write_le_u32(ptr + 8, op.id);
    ptr[12] = static_cast<uint8_t>(op.error_code);
    ptr[13] = static_cast<uint8_t>(op.error_flag);
    ptr[14] = op.is_sender ? protocol::WIRE_TRUE : protocol::WIRE_FALSE;
}

void Serializer::serialize(
    const protocol::Configuration& op,
    std::vector<uint8_t>& out
)
{
    const size_t offset = out.size();
    const size_t total_len = op.total_length();
    out.resize(offset + total_len);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::Configuration));
    write_le_u32(ptr + 4, static_cast<uint32_t>(total_len));
    ptr[8] = static_cast<uint8_t>(op.param_type);
    if (op.param_type == protocol::ConfigParamType::ArbitrationLostBehavior)
    {
        ptr[9] = static_cast<uint8_t>(op.arbitration_behavior);
    }
    else
    {
        write_le_u32(ptr + 9, op.baudrate);
    }
}

void Serializer::serialize(
    const protocol::Status& op,
    std::vector<uint8_t>& out
)
{
    const size_t offset = out.size();
    out.resize(offset + protocol::STATUS_SIZE);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::Status));
    write_le_u32(ptr + 4, static_cast<uint32_t>(protocol::STATUS_SIZE));
    ptr[8] = static_cast<uint8_t>(op.status);
}

void Serializer::serialize(
    const protocol::Wakeup& /*op*/,
    std::vector<uint8_t>& out
)
{
    const size_t offset = out.size();
    out.resize(offset + protocol::WAKEUP_SIZE);
    uint8_t* ptr = out.data() + offset;

    write_le_u32(ptr, static_cast<uint32_t>(protocol::OpCode::Wakeup));
    write_le_u32(ptr + 4, static_cast<uint32_t>(protocol::WAKEUP_SIZE));
}

void Serializer::serialize(
    const protocol::BusOperation& op,
    std::vector<uint8_t>& out
)
{
    std::visit(
        [&out](const auto& concrete_op) { serialize(concrete_op, out); },
        op
    );
}

std::vector<uint8_t> Serializer::serialize(const protocol::BusOperation& op)
{
    std::vector<uint8_t> result;
    serialize(op, result);
    return result;
}

void Serializer::serialize_batch(
    const std::vector<protocol::BusOperation>& ops,
    std::vector<uint8_t>& out
)
{
    for (const auto& op : ops)
    {
        serialize(op, out);
    }
}

std::vector<uint8_t> Serializer::serialize_batch(
    const std::vector<protocol::BusOperation>& ops
)
{
    std::vector<uint8_t> result;
    serialize_batch(ops, result);
    return result;
}

}  // namespace fmi_ls_bus::codec
