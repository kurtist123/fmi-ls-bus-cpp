#include "fmi_ls_bus/channel/can_channel.hpp"

#include "fmi_ls_bus/codec/deserializer.hpp"
#include "fmi_ls_bus/codec/serializer.hpp"

namespace fmi_ls_bus::channel
{

CanChannel::CanChannel(CanChannelConfig config)
    : config_(std::move(config))
    , status_(config_.initial_status)
    , tx_queue_(config_.queue_capacity)
    , rx_queue_(config_.queue_capacity)
{
}

bool CanChannel::queue_transmit(const protocol::CanTransmit& frame)
{
    return queue_operation(frame);
}

bool CanChannel::queue_transmit(const protocol::CanFdTransmit& frame)
{
    return queue_operation(frame);
}

bool CanChannel::queue_transmit(const protocol::CanXlTransmit& frame)
{
    return queue_operation(frame);
}

bool CanChannel::queue_operation(const protocol::BusOperation& op)
{
    const protocol::OpCode code = protocol::get_op_code(op);

    // If node is in BusOff, transmissions are prohibited
    if (is_bus_off() && (code == protocol::OpCode::CanTransmit ||
                         code == protocol::OpCode::CanFdTransmit ||
                         code == protocol::OpCode::CanXlTransmit))
    {
        return false;
    }

    const size_t op_len = protocol::get_total_length(op);
    if (queued_tx_bytes_ + op_len > config_.max_buffer_size)
    {
        return false;
    }

    if (tx_queue_.push(op))
    {
        queued_tx_bytes_ += op_len;
        return true;
    }
    return false;
}

bool CanChannel::has_pending_tx() const
{
    return !tx_queue_.empty();
}

size_t CanChannel::pending_tx_count() const
{
    return tx_queue_.size();
}

size_t CanChannel::estimated_tx_bytes() const
{
    return queued_tx_bytes_;
}

std::vector<uint8_t> CanChannel::pack_tx_buffer()
{
    std::vector<protocol::BusOperation> ops = tx_queue_.drain();
    queued_tx_bytes_ = 0;
    std::vector<uint8_t> buffer;
    codec::Serializer::serialize_batch(ops, buffer);
    return buffer;
}

size_t CanChannel::unpack_rx_buffer(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0)
    {
        return 0;
    }

    std::vector<protocol::BusOperation> ops =
        codec::Deserializer::deserialize_all(data, size);

    size_t count = 0;
    for (auto& op : ops)
    {
        const protocol::OpCode code = protocol::get_op_code(op);

        // Filter notifications if disabled per spec
        if (!config_.bus_notifications)
        {
            if (code == protocol::OpCode::Confirm ||
                code == protocol::OpCode::ArbitrationLost ||
                code == protocol::OpCode::BusError)
            {
                continue;
            }
        }

        // If incoming operation is Status, update local status
        if (code == protocol::OpCode::Status)
        {
            const auto& st = std::get<protocol::Status>(op);
            status_ = st.status;
        }

        if (rx_queue_.push(std::move(op)))
        {
            ++count;
        }
    }
    return count;
}

size_t CanChannel::unpack_rx_buffer(const std::vector<uint8_t>& buffer)
{
    return unpack_rx_buffer(buffer.data(), buffer.size());
}

bool CanChannel::has_pending_rx() const
{
    return !rx_queue_.empty();
}

size_t CanChannel::pending_rx_count() const
{
    return rx_queue_.size();
}

std::optional<protocol::BusOperation> CanChannel::pop_rx_operation()
{
    return rx_queue_.pop();
}

std::vector<protocol::BusOperation> CanChannel::drain_rx_operations()
{
    return rx_queue_.drain();
}

void CanChannel::clear_tx()
{
    tx_queue_.clear();
    queued_tx_bytes_ = 0;
}

void CanChannel::clear_rx()
{
    rx_queue_.clear();
}

void CanChannel::reset()
{
    clear_tx();
    clear_rx();
    status_ = config_.initial_status;
}

}  // namespace fmi_ls_bus::channel
