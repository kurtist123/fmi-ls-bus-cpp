/**
 * @file can_channel.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief CAN endpoint channel managing TX/RX queues and step batching
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

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

/**
 * @brief Configuration parameters for a CanChannel instance
 */
struct CanChannelConfig
{
    std::string name{"CAN"};
    size_t max_buffer_size{2'048};
    bool bus_notifications{false};
    protocol::NodeStatus initial_status{protocol::NodeStatus::ErrorActive};
    size_t queue_capacity{1'024};
};

/**
 * @brief Manages CAN communication channel endpoints with TX/RX queues and step
 * batching
 */
class CanChannel
{
public:
    /**
     * @brief Constructs a CanChannel with the specified configuration
     *
     * @param config Configuration parameters for buffers, capacities, and node
     * status
     */
    explicit CanChannel(CanChannelConfig config = {});

    /**
     * @brief Returns the configured channel name
     *
     * @return Channel name string
     */
    [[nodiscard]]
    const std::string& name() const noexcept
    {
        return config_.name;
    }

    /**
     * @brief Returns maximum byte size allowed for packed wire buffers
     *
     * @return Maximum buffer capacity in bytes
     */
    [[nodiscard]]
    size_t max_buffer_size() const noexcept
    {
        return config_.max_buffer_size;
    }

    /**
     * @brief Checks whether bus notifications (Confirm, BusError, etc.) are
     * enabled
     *
     * @return True if bus notifications are active, false otherwise
     */
    [[nodiscard]]
    bool notifications_enabled() const noexcept
    {
        return config_.bus_notifications;
    }

    /**
     * @brief Enables or disables handling of bus notifications
     *
     * @param enabled Flag indicating whether bus notifications should be
     * processed
     */
    void set_notifications_enabled(bool enabled) noexcept
    {
        config_.bus_notifications = enabled;
    }

    /**
     * @brief Returns current node error status
     *
     * @return Current NodeStatus enum value
     */
    [[nodiscard]]
    protocol::NodeStatus status() const noexcept
    {
        return status_;
    }

    /**
     * @brief Updates node error status
     *
     * @param new_status New NodeStatus to apply
     */
    void set_status(protocol::NodeStatus new_status) noexcept
    {
        status_ = new_status;
    }

    /**
     * @brief Checks if channel is currently in BusOff state
     *
     * @return True if status is BusOff, false otherwise
     */
    [[nodiscard]]
    bool is_bus_off() const noexcept
    {
        return status_ == protocol::NodeStatus::BusOff;
    }

    /**
     * @brief Queues a classic CAN transmit frame
     *
     * @param frame CanTransmit frame to enqueue
     * @return True if queued successfully, false if queue full or node is
     * BusOff
     */
    bool queue_transmit(const protocol::CanTransmit& frame);

    /**
     * @brief Queues a CAN-FD transmit frame
     *
     * @param frame CanFdTransmit frame to enqueue
     * @return True if queued successfully, false if queue full or node is
     * BusOff
     */
    bool queue_transmit(const protocol::CanFdTransmit& frame);

    /**
     * @brief Queues a CAN-XL transmit frame
     *
     * @param frame CanXlTransmit frame to enqueue
     * @return True if queued successfully, false if queue full or node is
     * BusOff
     */
    bool queue_transmit(const protocol::CanXlTransmit& frame);

    /**
     * @brief Queues an arbitrary bus operation
     *
     * @param op BusOperation variant to enqueue
     * @return True if queued successfully, false if queue full
     */
    bool queue_operation(const protocol::BusOperation& op);

    /**
     * @brief Checks if transmit queue contains pending operations
     *
     * @return True if transmit operations are pending, false otherwise
     */
    [[nodiscard]]
    bool has_pending_tx() const;

    /**
     * @brief Returns number of pending operations in transmit queue
     *
     * @return Count of queued transmit operations
     */
    [[nodiscard]]
    size_t pending_tx_count() const;

    /**
     * @brief Returns estimated total serialized byte size of pending transmit
     * operations
     *
     * @return Estimated serialized byte size
     */
    [[nodiscard]]
    size_t estimated_tx_bytes() const;

    /**
     * @brief Serializes all pending transmit operations into a single wire byte
     * buffer
     *
     * @return Serialized byte buffer for transmission over FMI 3.0 Binary
     * variable
     */
    std::vector<uint8_t> pack_tx_buffer();

    /**
     * @brief Deserializes operations from a raw byte buffer and enqueues into
     * RX queue
     *
     * @param data Pointer to raw received byte buffer
     * @param size Number of available bytes
     * @return Number of operations successfully unpacked and enqueued
     */
    size_t unpack_rx_buffer(const uint8_t* data, size_t size);

    /**
     * @brief Deserializes operations from a byte vector and enqueues into RX
     * queue
     *
     * @param buffer Received byte vector
     * @return Number of operations successfully unpacked and enqueued
     */
    size_t unpack_rx_buffer(const std::vector<uint8_t>& buffer);

    /**
     * @brief Checks if receive queue contains pending operations
     *
     * @return True if receive operations are pending, false otherwise
     */
    [[nodiscard]]
    bool has_pending_rx() const;

    /**
     * @brief Returns number of pending operations in receive queue
     *
     * @return Count of queued receive operations
     */
    [[nodiscard]]
    size_t pending_rx_count() const;

    /**
     * @brief Pops and returns the next pending operation from receive queue
     *
     * @return Optional containing next operation, or std::nullopt if queue is
     * empty
     */
    std::optional<protocol::BusOperation> pop_rx_operation();

    /**
     * @brief Drains all pending operations from receive queue
     *
     * @return Vector containing all drained operations
     */
    std::vector<protocol::BusOperation> drain_rx_operations();

    /**
     * @brief Clears all pending operations in transmit queue
     */
    void clear_tx();

    /**
     * @brief Clears all pending operations in receive queue
     */
    void clear_rx();

    /**
     * @brief Resets channel queues and restores initial node status
     */
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
