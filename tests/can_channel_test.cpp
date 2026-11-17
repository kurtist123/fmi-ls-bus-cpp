/**
 * @file can_channel_test.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Unit tests for CAN channel queuing, batching, and overflow policies
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include <gtest/gtest.h>

#include "fmi_ls_bus/channel/can_channel.hpp"
#include "fmi_ls_bus/codec/deserializer.hpp"
#include "fmi_ls_bus/codec/serializer.hpp"

using namespace fmi_ls_bus;
using namespace fmi_ls_bus::channel;
using namespace fmi_ls_bus::protocol;
using namespace fmi_ls_bus::codec;

TEST(CanChannelTest, BasicTransmitAndPack)
{
    CanChannelConfig cfg;
    cfg.name = "CAN1";
    cfg.max_buffer_size = 1'024;
    CanChannel channel(cfg);

    EXPECT_FALSE(channel.has_pending_tx());
    EXPECT_EQ(channel.pending_tx_count(), 0);

    CanTransmit tx1;
    tx1.id = 0x100;
    tx1.data = {0x01, 0x02, 0x03};
    EXPECT_TRUE(channel.queue_transmit(tx1));

    CanFdTransmit tx2;
    tx2.id = 0x200;
    tx2.data = {0x0A, 0x0B};
    EXPECT_TRUE(channel.queue_transmit(tx2));

    EXPECT_TRUE(channel.has_pending_tx());
    EXPECT_EQ(channel.pending_tx_count(), 2);
    EXPECT_GT(channel.estimated_tx_bytes(), 0);

    std::vector<uint8_t> buffer = channel.pack_tx_buffer();
    EXPECT_FALSE(buffer.empty());
    EXPECT_FALSE(channel.has_pending_tx());
    EXPECT_EQ(channel.pending_tx_count(), 0);
    EXPECT_EQ(channel.estimated_tx_bytes(), 0);

    // Verify deserializing buffer
    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 2);
    EXPECT_EQ(std::get<CanTransmit>(ops[0]), tx1);
    EXPECT_EQ(std::get<CanFdTransmit>(ops[1]), tx2);
}

TEST(CanChannelTest, ReceiveBufferUnpacking)
{
    CanChannel channel;

    std::vector<BusOperation> sent_ops;
    CanTransmit tx;
    tx.id = 0x111;
    tx.data = {0xAA, 0xBB};
    sent_ops.push_back(tx);

    std::vector<uint8_t> buffer = Serializer::serialize_batch(sent_ops);

    size_t count = channel.unpack_rx_buffer(buffer);
    EXPECT_EQ(count, 1);
    EXPECT_TRUE(channel.has_pending_rx());

    auto received = channel.pop_rx_operation();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(std::get<CanTransmit>(*received), tx);
    EXPECT_FALSE(channel.has_pending_rx());
}

TEST(CanChannelTest, NotificationFiltering)
{
    // Channel with bus_notifications = false (default)
    CanChannelConfig cfg_no_notify;
    cfg_no_notify.bus_notifications = false;
    CanChannel channel_no_notify(cfg_no_notify);

    std::vector<BusOperation> ops;
    ops.push_back(Confirm{0x100});
    ops.push_back(ArbitrationLost{0x100});
    ops.push_back(
        BusError{0x100, BusErrorCode::AckError, ErrorFlag::Primary, false}
    );
    CanTransmit data_frame;
    data_frame.id = 0x200;
    data_frame.data = {0x12};
    ops.push_back(data_frame);

    std::vector<uint8_t> raw = Serializer::serialize_batch(ops);

    size_t received_count = channel_no_notify.unpack_rx_buffer(raw);
    EXPECT_EQ(
        received_count,
        1
    );  // Only data_frame, notifications filtered out
    auto received = channel_no_notify.drain_rx_operations();
    ASSERT_EQ(received.size(), 1);
    EXPECT_EQ(std::get<CanTransmit>(received[0]), data_frame);

    // Channel with bus_notifications = true
    CanChannelConfig cfg_notify;
    cfg_notify.bus_notifications = true;
    CanChannel channel_notify(cfg_notify);

    size_t received_count_all = channel_notify.unpack_rx_buffer(raw);
    EXPECT_EQ(received_count_all, 4);
    auto received_all = channel_notify.drain_rx_operations();
    ASSERT_EQ(received_all.size(), 4);
}

TEST(CanChannelTest, BusOffBehavior)
{
    CanChannel channel;
    EXPECT_FALSE(channel.is_bus_off());

    CanTransmit tx;
    tx.id = 0x123;
    tx.data = {1, 2};
    EXPECT_TRUE(channel.queue_transmit(tx));

    // Transition to BusOff
    channel.set_status(NodeStatus::BusOff);
    EXPECT_TRUE(channel.is_bus_off());

    // Transmit should be rejected
    EXPECT_FALSE(channel.queue_transmit(tx));

    // Receiving Status operation updates internal state
    Status st_active{NodeStatus::ErrorActive};
    std::vector<uint8_t> st_buf = Serializer::serialize(st_active);
    channel.unpack_rx_buffer(st_buf);
    EXPECT_FALSE(channel.is_bus_off());
    EXPECT_EQ(channel.status(), NodeStatus::ErrorActive);

    // Can transmit again
    EXPECT_TRUE(channel.queue_transmit(tx));
}

TEST(CanChannelTest, CapacityBounds)
{
    CanChannelConfig cfg;
    cfg.max_buffer_size = 50;  // Very small buffer
    CanChannel channel(cfg);

    CanTransmit tx1;
    tx1.id = 0x01;
    tx1.data = {1, 2, 3, 4, 5, 6, 7, 8};  // 16 + 8 = 24 bytes
    EXPECT_TRUE(channel.queue_transmit(tx1));

    CanTransmit tx2;
    tx2.id = 0x02;
    tx2.data = {1, 2, 3, 4, 5, 6, 7, 8};  // 24 bytes (total 48 <= 50)
    EXPECT_TRUE(channel.queue_transmit(tx2));

    CanTransmit tx3;
    tx3.id = 0x03;
    tx3.data = {1, 2};  // 16 + 2 = 18 bytes (total 48 + 18 = 66 > 50)
    EXPECT_FALSE(
        channel.queue_transmit(tx3)
    );  // Must reject to prevent buffer overflow

    EXPECT_EQ(channel.pending_tx_count(), 2);
}

TEST(CanChannelTest, ResetAndClear)
{
    CanChannel channel;
    channel.queue_transmit(CanTransmit{0x10, false, false, 0, {1}});
    EXPECT_TRUE(channel.has_pending_tx());

    channel.clear_tx();
    EXPECT_FALSE(channel.has_pending_tx());
    EXPECT_EQ(channel.estimated_tx_bytes(), 0);

    channel.set_status(NodeStatus::BusOff);
    channel.reset();
    EXPECT_EQ(channel.status(), NodeStatus::ErrorActive);
}
