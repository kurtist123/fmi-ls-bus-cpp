#ifndef FMI_LS_BUS_CHANNEL_CAN_CHANNEL_HPP
#define FMI_LS_BUS_CHANNEL_CAN_CHANNEL_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "fmi_ls_bus/channel/frame_queue.hpp"
#include "fmi_ls_bus/protocol/constants.hpp"
#include "fmi_ls_bus/protocol/operations.hpp"

namespace fmi_ls_bus::channel
{

struct CanChannelConfig
{
    std::string name{"CAN"};
    size_t max_buffer_size{2'048};
    bool bus_notifications{false};
    protocol::NodeStatus initial_status{protocol::NodeStatus::ErrorActive};
    size_t queue_capacity{1'024};
};

class CanChannel
{
public:
    explicit CanChannel(CanChannelConfig config = {});

    [[nodiscard]]
    const std::string& name() const noexcept
    {
        return config_.name;
    }

    [[nodiscard]]
    size_t max_buffer_size() const noexcept
    {
        return config_.max_buffer_size;
    }

    [[nodiscard]]
    bool notifications_enabled() const noexcept
    {
        return config_.bus_notifications;
    }

    void set_notifications_enabled(bool enabled) noexcept
    {
        config_.bus_notifications = enabled;
    }

    [[nodiscard]]
    protocol::NodeStatus status() const noexcept
    {
        return status_;
    }

    void set_status(protocol::NodeStatus new_status) noexcept
    {
        status_ = new_status;
    }

    [[nodiscard]]
    bool is_bus_off() const noexcept
    {
        return status_ == protocol::NodeStatus::BusOff;
    }

    // Transmit queue operations
    bool queue_transmit(const protocol::CanTransmit& frame);
    bool queue_transmit(const protocol::CanFdTransmit& frame);
    bool queue_transmit(const protocol::CanXlTransmit& frame);
    bool queue_operation(const protocol::BusOperation& op);

    [[nodiscard]]
    bool has_pending_tx() const;
    [[nodiscard]]
    size_t pending_tx_count() const;
    [[nodiscard]]
    size_t estimated_tx_bytes() const;

    // Serialization into Tx buffer for FMI 3.0 Binary variable
    std::vector<uint8_t> pack_tx_buffer();

    // Deserialization from Rx buffer into Rx queue
    size_t unpack_rx_buffer(const uint8_t* data, size_t size);
    size_t unpack_rx_buffer(const std::vector<uint8_t>& buffer);

    // Receive queue operations
    [[nodiscard]]
    bool has_pending_rx() const;
    [[nodiscard]]
    size_t pending_rx_count() const;
    std::optional<protocol::BusOperation> pop_rx_operation();
    std::vector<protocol::BusOperation> drain_rx_operations();

    void clear_tx();
    void clear_rx();
    void reset();

private:
    CanChannelConfig config_;
    protocol::NodeStatus status_{protocol::NodeStatus::ErrorActive};
    size_t queued_tx_bytes_{0};
    FrameQueue<protocol::BusOperation> tx_queue_;
    FrameQueue<protocol::BusOperation> rx_queue_;
};

}  // namespace fmi_ls_bus::channel

#endif  // FMI_LS_BUS_CHANNEL_CAN_CHANNEL_HPP
