#ifndef FMI_LS_BUS_PROTOCOL_OPERATIONS_HPP
#define FMI_LS_BUS_PROTOCOL_OPERATIONS_HPP

#include <cstdint>
#include <variant>
#include <vector>

#include "fmi_ls_bus/protocol/constants.hpp"

namespace fmi_ls_bus::protocol
{

struct FormatError
{
    std::vector<uint8_t> raw_data;

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return FORMAT_ERROR_FIXED_SIZE + raw_data.size();
    }

    bool operator== (const FormatError& other) const noexcept
    {
        return raw_data == other.raw_data;
    }
};

struct CanTransmit
{
    uint32_t id{0};
    bool extended_id{false};
    bool remote_frame{false};
    uint16_t requested_rtr_length{0};  // For RTR frames where payload is empty
    std::vector<uint8_t> data;

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        // For RTR, wire dataLength = requested_rtr_length, but 0 payload bytes
        return CAN_TRANSMIT_FIXED_SIZE + (remote_frame ? 0 : data.size());
    }

    bool operator== (const CanTransmit& other) const noexcept
    {
        return id == other.id && extended_id == other.extended_id &&
               remote_frame == other.remote_frame &&
               requested_rtr_length == other.requested_rtr_length &&
               data == other.data;
    }
};

struct CanFdTransmit
{
    uint32_t id{0};
    bool extended_id{false};
    bool bit_rate_switch{false};
    bool error_state_indicator{false};
    std::vector<uint8_t> data;

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return CAN_FD_TRANSMIT_FIXED_SIZE + data.size();
    }

    bool operator== (const CanFdTransmit& other) const noexcept
    {
        return id == other.id && extended_id == other.extended_id &&
               bit_rate_switch == other.bit_rate_switch &&
               error_state_indicator == other.error_state_indicator &&
               data == other.data;
    }
};

struct CanXlTransmit
{
    uint32_t id{0};
    bool extended_id{false};
    bool simple_extended_content{false};
    uint8_t sdu_type{0};
    uint8_t vcid{0};
    uint32_t acceptance_field{0};
    std::vector<uint8_t> data;

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return CAN_XL_TRANSMIT_FIXED_SIZE + data.size();
    }

    bool operator== (const CanXlTransmit& other) const noexcept
    {
        return id == other.id && extended_id == other.extended_id &&
               simple_extended_content == other.simple_extended_content &&
               sdu_type == other.sdu_type && vcid == other.vcid &&
               acceptance_field == other.acceptance_field && data == other.data;
    }
};

struct Confirm
{
    uint32_t id{0};

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return CONFIRM_SIZE;
    }

    bool operator== (const Confirm& other) const noexcept
    {
        return id == other.id;
    }
};

struct ArbitrationLost
{
    uint32_t id{0};

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return ARBITRATION_LOST_SIZE;
    }

    bool operator== (const ArbitrationLost& other) const noexcept
    {
        return id == other.id;
    }
};

struct BusError
{
    uint32_t id{0};
    BusErrorCode error_code{BusErrorCode::BitError};
    ErrorFlag error_flag{ErrorFlag::Primary};
    bool is_sender{false};

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return BUS_ERROR_SIZE;
    }

    bool operator== (const BusError& other) const noexcept
    {
        return id == other.id && error_code == other.error_code &&
               error_flag == other.error_flag && is_sender == other.is_sender;
    }
};

struct Configuration
{
    ConfigParamType param_type{ConfigParamType::CanBaudrate};
    uint32_t baudrate{500'000};
    ArbitrationLostBehavior arbitration_behavior{
        ArbitrationLostBehavior::BufferAndRetransmit
    };

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        if (param_type == ConfigParamType::ArbitrationLostBehavior)
        {
            return CONFIGURATION_ARB_SIZE;
        }
        return CONFIGURATION_BAUDRATE_SIZE;
    }

    bool operator== (const Configuration& other) const noexcept
    {
        if (param_type != other.param_type)
        {
            return false;
        }
        if (param_type == ConfigParamType::ArbitrationLostBehavior)
        {
            return arbitration_behavior == other.arbitration_behavior;
        }
        return baudrate == other.baudrate;
    }
};

struct Status
{
    NodeStatus status{NodeStatus::ErrorActive};

    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return STATUS_SIZE;
    }

    bool operator== (const Status& other) const noexcept
    {
        return status == other.status;
    }
};

struct Wakeup
{
    [[nodiscard]]
    size_t total_length() const noexcept
    {
        return WAKEUP_SIZE;
    }

    bool operator== (const Wakeup&) const noexcept
    {
        return true;
    }
};

using BusOperation = std::variant<
    FormatError,
    CanTransmit,
    CanFdTransmit,
    CanXlTransmit,
    Confirm,
    ArbitrationLost,
    BusError,
    Configuration,
    Status,
    Wakeup>;

inline OpCode get_op_code(const BusOperation& op) noexcept
{
    return std::visit(
        [](const auto& o) -> OpCode {
            using T = std::decay_t<decltype(o)>;
            if constexpr (std::is_same_v<T, FormatError>)
            {
                return OpCode::FormatError;
            }
            else if constexpr (std::is_same_v<T, CanTransmit>)
            {
                return OpCode::CanTransmit;
            }
            else if constexpr (std::is_same_v<T, CanFdTransmit>)
            {
                return OpCode::CanFdTransmit;
            }
            else if constexpr (std::is_same_v<T, CanXlTransmit>)
            {
                return OpCode::CanXlTransmit;
            }
            else if constexpr (std::is_same_v<T, Confirm>)
            {
                return OpCode::Confirm;
            }
            else if constexpr (std::is_same_v<T, ArbitrationLost>)
            {
                return OpCode::ArbitrationLost;
            }
            else if constexpr (std::is_same_v<T, BusError>)
            {
                return OpCode::BusError;
            }
            else if constexpr (std::is_same_v<T, Configuration>)
            {
                return OpCode::Configuration;
            }
            else if constexpr (std::is_same_v<T, Status>)
            {
                return OpCode::Status;
            }
            else if constexpr (std::is_same_v<T, Wakeup>)
            {
                return OpCode::Wakeup;
            }
        },
        op
    );
}

inline size_t get_total_length(const BusOperation& op) noexcept
{
    return std::visit([](const auto& o) { return o.total_length(); }, op);
}

}  // namespace fmi_ls_bus::protocol

#endif  // FMI_LS_BUS_PROTOCOL_OPERATIONS_HPP
