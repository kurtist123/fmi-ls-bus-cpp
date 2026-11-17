/**
 * @file deserializer.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Implementation of binary wire deserializer for FMI LS-BUS
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include "fmi_ls_bus/codec/deserializer.hpp"

#include <cstring>

#include "fmi_ls_bus/codec/endian.hpp"
#include "fmi_ls_bus/protocol/constants.hpp"

namespace fmi_ls_bus::codec
{

DeserializationResult Deserializer::deserialize_next(
    const uint8_t* data,
    size_t size
)
{
    DeserializationResult result;

    if (data == nullptr || size < protocol::OPERATION_HEADER_SIZE)
    {
        result.success = false;
        result.bytes_consumed = 0;
        protocol::FormatError fe;
        if (data != nullptr && size > 0)
        {
            fe.raw_data.assign(data, data + size);
        }
        result.operation = std::move(fe);
        result.error_message =
            "Buffer too small to contain operation header (needs at least 8 "
            "bytes)";
        return result;
    }

    const uint32_t op_code_raw = read_le_u32(data);
    const uint32_t length = read_le_u32(data + 4);

    if (length < protocol::OPERATION_HEADER_SIZE)
    {
        result.success = false;
        result.bytes_consumed = size;
        protocol::FormatError fe;
        fe.raw_data.assign(data, data + size);
        result.operation = fe;
        result.error_message =
            "Invalid operation header length field (< 8 bytes)";
        return result;
    }

    if (length > size)
    {
        result.success = false;
        result.bytes_consumed = size;
        protocol::FormatError fe;
        fe.raw_data.assign(data, data + size);
        result.operation = fe;
        result.error_message =
            "Operation length exceeds available buffer size (truncated buffer)";
        return result;
    }

    switch (static_cast<protocol::OpCode>(op_code_raw))
    {
        case protocol::OpCode::FormatError:
            {
                if (length < protocol::FORMAT_ERROR_FIXED_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "FormatError length smaller than fixed size 10";
                    return result;
                }
                const uint16_t data_len = read_le_u16(data + 8);
                if (protocol::FORMAT_ERROR_FIXED_SIZE + data_len > length)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "FormatError payload length exceeds total length";
                    return result;
                }
                protocol::FormatError op;
                op.raw_data.assign(data + 10, data + 10 + data_len);
                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::CanTransmit:
            {
                if (length < protocol::CAN_TRANSMIT_FIXED_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "CanTransmit length smaller than fixed size 16";
                    return result;
                }
                protocol::CanTransmit op;
                op.id = read_le_u32(data + 8);
                op.extended_id = (data[12] != 0);
                op.remote_frame = (data[13] != 0);
                const uint16_t wire_len = read_le_u16(data + 14);

                if (op.remote_frame)
                {
                    op.requested_rtr_length = wire_len;
                    op.data.clear();
                }
                else
                {
                    if (protocol::CAN_TRANSMIT_FIXED_SIZE + wire_len > length)
                    {
                        result.success = false;
                        result.bytes_consumed = length;
                        protocol::FormatError fe;
                        fe.raw_data.assign(data, data + length);
                        result.operation = std::move(fe);
                        result.error_message =
                            "CanTransmit payload exceeds operation length";
                        return result;
                    }
                    op.requested_rtr_length = 0;
                    op.data.assign(data + 16, data + 16 + wire_len);
                }

                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::CanFdTransmit:
            {
                if (length < protocol::CAN_FD_TRANSMIT_FIXED_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "CanFdTransmit length smaller than fixed size 17";
                    return result;
                }
                protocol::CanFdTransmit op;
                op.id = read_le_u32(data + 8);
                op.extended_id = (data[12] != 0);
                op.bit_rate_switch = (data[13] != 0);
                op.error_state_indicator = (data[14] != 0);
                const uint16_t wire_len = read_le_u16(data + 15);

                if (protocol::CAN_FD_TRANSMIT_FIXED_SIZE + wire_len > length)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "CanFdTransmit payload exceeds operation length";
                    return result;
                }
                op.data.assign(data + 17, data + 17 + wire_len);

                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::CanXlTransmit:
            {
                if (length < protocol::CAN_XL_TRANSMIT_FIXED_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "CanXlTransmit length smaller than fixed size 22";
                    return result;
                }
                protocol::CanXlTransmit op;
                op.id = read_le_u32(data + 8);
                op.extended_id = (data[12] != 0);
                op.simple_extended_content = (data[13] != 0);
                op.sdu_type = data[14];
                op.vcid = data[15];
                op.acceptance_field = read_le_u32(data + 16);
                const uint16_t wire_len = read_le_u16(data + 20);

                if (protocol::CAN_XL_TRANSMIT_FIXED_SIZE + wire_len > length)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "CanXlTransmit payload exceeds operation length";
                    return result;
                }
                op.data.assign(data + 22, data + 22 + wire_len);

                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::Confirm:
            {
                if (length < protocol::CONFIRM_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message = "Confirm length smaller than 12";
                    return result;
                }
                protocol::Confirm op;
                op.id = read_le_u32(data + 8);
                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::ArbitrationLost:
            {
                if (length < protocol::ARBITRATION_LOST_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "ArbitrationLost length smaller than 12";
                    return result;
                }
                protocol::ArbitrationLost op;
                op.id = read_le_u32(data + 8);
                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::BusError:
            {
                if (length < protocol::BUS_ERROR_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message = "BusError length smaller than 15";
                    return result;
                }
                protocol::BusError op;
                op.id = read_le_u32(data + 8);
                op.error_code = static_cast<protocol::BusErrorCode>(data[12]);
                op.error_flag = static_cast<protocol::ErrorFlag>(data[13]);
                op.is_sender = (data[14] != 0);
                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::Configuration:
            {
                if (length < protocol::CONFIGURATION_ARB_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message =
                        "Configuration length smaller than 10";
                    return result;
                }
                protocol::Configuration op;
                op.param_type = static_cast<protocol::ConfigParamType>(data[8]);
                if (op.param_type ==
                    protocol::ConfigParamType::ArbitrationLostBehavior)
                {
                    op.arbitration_behavior =
                        static_cast<protocol::ArbitrationLostBehavior>(data[9]);
                }
                else
                {
                    if (length < protocol::CONFIGURATION_BAUDRATE_SIZE)
                    {
                        result.success = false;
                        result.bytes_consumed = length;
                        protocol::FormatError fe;
                        fe.raw_data.assign(data, data + length);
                        result.operation = std::move(fe);
                        result.error_message =
                            "Configuration baudrate length smaller than 13";
                        return result;
                    }
                    op.baudrate = read_le_u32(data + 9);
                }
                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::Status:
            {
                if (length < protocol::STATUS_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message = "Status length smaller than 9";
                    return result;
                }
                protocol::Status op;
                op.status = static_cast<protocol::NodeStatus>(data[8]);
                result.success = true;
                result.bytes_consumed = length;
                result.operation = op;
                return result;
            }

        case protocol::OpCode::Wakeup:
            {
                if (length < protocol::WAKEUP_SIZE)
                {
                    result.success = false;
                    result.bytes_consumed = length;
                    protocol::FormatError fe;
                    fe.raw_data.assign(data, data + length);
                    result.operation = std::move(fe);
                    result.error_message = "Wakeup length smaller than 8";
                    return result;
                }
                result.success = true;
                result.bytes_consumed = length;
                result.operation = protocol::Wakeup{};
                return result;
            }

        default:
            {
                // Unrecognized opcode -> emit FormatError containing this
                // operation's bytes
                protocol::FormatError fe;
                fe.raw_data.assign(data, data + length);
                result.success = false;
                result.bytes_consumed = length;
                result.operation = fe;
                result.error_message = "Unrecognized operation opcode: " +
                                       std::to_string(op_code_raw);
                return result;
            }
    }
}

std::vector<protocol::BusOperation> Deserializer::deserialize_all(
    const uint8_t* data,
    size_t size
)
{
    if (data == nullptr || size == 0)
    {
        return {};
    }

    std::vector<protocol::BusOperation> ops;
    size_t offset = 0;

    while (offset < size)
    {
        const size_t remaining = size - offset;
        DeserializationResult res = deserialize_next(data + offset, remaining);
        if (res.success)
        {
            ops.push_back(std::move(res.operation));
            offset += res.bytes_consumed;
        }
        else
        {
            // If bytes consumed is valid, emit format error if present
            if (std::holds_alternative<protocol::FormatError>(res.operation))
            {
                ops.push_back(std::move(res.operation));
            }
            else
            {
                // Emit FormatError with remaining bytes
                protocol::FormatError fe;
                fe.raw_data.assign(data + offset, data + size);
                ops.push_back(std::move(fe));
            }
            break;
        }
    }

    return ops;
}

std::vector<protocol::BusOperation> Deserializer::deserialize_all(
    const std::vector<uint8_t>& buffer
)
{
    return deserialize_all(buffer.data(), buffer.size());
}

}  // namespace fmi_ls_bus::codec
