#include <gtest/gtest.h>

#include "fmi_ls_bus/codec/deserializer.hpp"
#include "fmi_ls_bus/codec/endian.hpp"
#include "fmi_ls_bus/codec/serializer.hpp"
#include "fmi_ls_bus/protocol/constants.hpp"
#include "fmi_ls_bus/protocol/operations.hpp"

using namespace fmi_ls_bus;
using namespace fmi_ls_bus::protocol;
using namespace fmi_ls_bus::codec;

TEST(EndianTest, LittleEndianReadWrite)
{
    uint8_t buffer[8] = {0};

    write_le_u16(buffer, 0x1234);
    EXPECT_EQ(buffer[0], 0x34);
    EXPECT_EQ(buffer[1], 0x12);
    EXPECT_EQ(read_le_u16(buffer), 0x1234);

    write_le_u32(buffer, 0xAABBCCDD);
    EXPECT_EQ(buffer[0], 0xDD);
    EXPECT_EQ(buffer[1], 0xCC);
    EXPECT_EQ(buffer[2], 0xBB);
    EXPECT_EQ(buffer[3], 0xAA);
    EXPECT_EQ(read_le_u32(buffer), 0xAABBCCDD);

    write_le_u64(buffer, 0x01020304'05060708ULL);
    EXPECT_EQ(buffer[0], 0x08);
    EXPECT_EQ(buffer[7], 0x01);
    EXPECT_EQ(read_le_u64(buffer), 0x01020304'05060708ULL);
}

TEST(CodecTest, CanTransmitStandard)
{
    CanTransmit tx;
    tx.id = 0x123;
    tx.extended_id = false;
    tx.remote_frame = false;
    tx.data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

    std::vector<uint8_t> buffer = Serializer::serialize(tx);
    ASSERT_EQ(buffer.size(), 24);  // 16 header + 8 payload

    // Verify wire bytes directly
    EXPECT_EQ(read_le_u32(buffer.data()), 0x0010);     // OpCode CanTransmit
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 24);     // Total Length
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x123);  // CAN ID
    EXPECT_EQ(buffer[12], 0);                          // Standard ID
    EXPECT_EQ(buffer[13], 0);                          // Not RTR
    EXPECT_EQ(read_le_u16(buffer.data() + 14), 8);     // DataLength

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<CanTransmit>(ops[0]));
    const auto& deserialized = std::get<CanTransmit>(ops[0]);
    EXPECT_EQ(deserialized, tx);
}

TEST(CodecTest, CanTransmitExtendedAndRtr)
{
    CanTransmit rtr;
    rtr.id = 0x18FF50A1;
    rtr.extended_id = true;
    rtr.remote_frame = true;
    rtr.requested_rtr_length = 4;

    std::vector<uint8_t> buffer = Serializer::serialize(rtr);
    ASSERT_EQ(buffer.size(), 16);  // 16 header + 0 payload

    EXPECT_EQ(read_le_u32(buffer.data()), 0x0010);
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 16);
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x18FF50A1);
    EXPECT_EQ(buffer[12], 1);                       // Extended
    EXPECT_EQ(buffer[13], 1);                       // RTR
    EXPECT_EQ(read_le_u16(buffer.data() + 14), 4);  // requested length

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<CanTransmit>(ops[0]));
    const auto& deserialized = std::get<CanTransmit>(ops[0]);
    EXPECT_EQ(deserialized, rtr);
    EXPECT_TRUE(deserialized.data.empty());
}

TEST(CodecTest, CanTransmitZeroPayload)
{
    CanTransmit tx;
    tx.id = 0x456;
    tx.extended_id = false;
    tx.remote_frame = false;
    tx.data.clear();

    std::vector<uint8_t> buffer = Serializer::serialize(tx);
    ASSERT_EQ(buffer.size(), 16);

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<CanTransmit>(ops[0]));
    EXPECT_EQ(std::get<CanTransmit>(ops[0]), tx);
}

TEST(CodecTest, CanFdTransmit)
{
    CanFdTransmit fd;
    fd.id = 0x200;
    fd.extended_id = false;
    fd.bit_rate_switch = true;
    fd.error_state_indicator = false;
    fd.data.resize(64);
    for (size_t i = 0; i < fd.data.size(); ++i)
    {
        fd.data[i] = static_cast<uint8_t>(i);
    }

    std::vector<uint8_t> buffer = Serializer::serialize(fd);
    ASSERT_EQ(buffer.size(), 17 + 64);  // 81 bytes

    EXPECT_EQ(read_le_u32(buffer.data()), 0x0011);
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 81);
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x200);
    EXPECT_EQ(buffer[12], 0);
    EXPECT_EQ(buffer[13], 1);  // BRS
    EXPECT_EQ(buffer[14], 0);  // ESI
    EXPECT_EQ(read_le_u16(buffer.data() + 15), 64);

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<CanFdTransmit>(ops[0]));
    EXPECT_EQ(std::get<CanFdTransmit>(ops[0]), fd);
}

TEST(CodecTest, CanXlTransmit)
{
    CanXlTransmit xl;
    xl.id = 0x7E0;
    xl.extended_id = true;
    xl.simple_extended_content = true;
    xl.sdu_type = 0x01;
    xl.vcid = 0x42;
    xl.acceptance_field = 0xDEADBEEF;
    xl.data.assign(256, 0xAB);

    std::vector<uint8_t> buffer = Serializer::serialize(xl);
    ASSERT_EQ(buffer.size(), 22 + 256);  // 278 bytes

    EXPECT_EQ(read_le_u32(buffer.data()), 0x0012);
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 278);
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x7E0);
    EXPECT_EQ(buffer[12], 1);                                // Extended
    EXPECT_EQ(buffer[13], 1);                                // SEC
    EXPECT_EQ(buffer[14], 0x01);                             // SDT
    EXPECT_EQ(buffer[15], 0x42);                             // VCID
    EXPECT_EQ(read_le_u32(buffer.data() + 16), 0xDEADBEEF);  // AF
    EXPECT_EQ(read_le_u16(buffer.data() + 20), 256);

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<CanXlTransmit>(ops[0]));
    EXPECT_EQ(std::get<CanXlTransmit>(ops[0]), xl);
}

TEST(CodecTest, Confirm)
{
    Confirm conf;
    conf.id = 0x321;

    std::vector<uint8_t> buffer = Serializer::serialize(conf);
    ASSERT_EQ(buffer.size(), 12);
    EXPECT_EQ(read_le_u32(buffer.data()), 0x0020);
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 12);
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x321);

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Confirm>(ops[0]));
    EXPECT_EQ(std::get<Confirm>(ops[0]), conf);
}

TEST(CodecTest, ArbitrationLost)
{
    ArbitrationLost arb;
    arb.id = 0x555;

    std::vector<uint8_t> buffer = Serializer::serialize(arb);
    ASSERT_EQ(buffer.size(), 12);
    EXPECT_EQ(read_le_u32(buffer.data()), 0x0030);
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 12);
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x555);

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<ArbitrationLost>(ops[0]));
    EXPECT_EQ(std::get<ArbitrationLost>(ops[0]), arb);
}

TEST(CodecTest, BusError)
{
    BusError err;
    err.id = 0x101;
    err.error_code = BusErrorCode::BitStuffingError;
    err.error_flag = ErrorFlag::Secondary;
    err.is_sender = true;

    std::vector<uint8_t> buffer = Serializer::serialize(err);
    ASSERT_EQ(buffer.size(), 15);
    EXPECT_EQ(read_le_u32(buffer.data()), 0x0031);
    EXPECT_EQ(read_le_u32(buffer.data() + 4), 15);
    EXPECT_EQ(read_le_u32(buffer.data() + 8), 0x101);
    EXPECT_EQ(buffer[12], 0x02);  // BitStuffingError
    EXPECT_EQ(buffer[13], 0x02);  // Secondary
    EXPECT_EQ(buffer[14], 0x01);  // isSender = true

    auto ops = Deserializer::deserialize_all(buffer);
    ASSERT_EQ(ops.size(), 1);
    ASSERT_TRUE(std::holds_alternative<BusError>(ops[0]));
    EXPECT_EQ(std::get<BusError>(ops[0]), err);
}

TEST(CodecTest, Configuration)
{
    // Baudrate test
    Configuration cfg_baud;
    cfg_baud.param_type = ConfigParamType::CanFdBaudrate;
    cfg_baud.baudrate = 2'000'000;

    std::vector<uint8_t> buf_baud = Serializer::serialize(cfg_baud);
    ASSERT_EQ(buf_baud.size(), 13);
    EXPECT_EQ(read_le_u32(buf_baud.data()), 0x0040);
    EXPECT_EQ(read_le_u32(buf_baud.data() + 4), 13);
    EXPECT_EQ(buf_baud[8], 0x02);
    EXPECT_EQ(read_le_u32(buf_baud.data() + 9), 2'000'000);

    auto ops_baud = Deserializer::deserialize_all(buf_baud);
    ASSERT_EQ(ops_baud.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Configuration>(ops_baud[0]));
    EXPECT_EQ(std::get<Configuration>(ops_baud[0]), cfg_baud);

    // Arbitration behavior test
    Configuration cfg_arb;
    cfg_arb.param_type = ConfigParamType::ArbitrationLostBehavior;
    cfg_arb.arbitration_behavior = ArbitrationLostBehavior::DiscardAndNotify;

    std::vector<uint8_t> buf_arb = Serializer::serialize(cfg_arb);
    ASSERT_EQ(buf_arb.size(), 10);
    EXPECT_EQ(read_le_u32(buf_arb.data()), 0x0040);
    EXPECT_EQ(read_le_u32(buf_arb.data() + 4), 10);
    EXPECT_EQ(buf_arb[8], 0x04);
    EXPECT_EQ(buf_arb[9], 0x02);

    auto ops_arb = Deserializer::deserialize_all(buf_arb);
    ASSERT_EQ(ops_arb.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Configuration>(ops_arb[0]));
    EXPECT_EQ(std::get<Configuration>(ops_arb[0]), cfg_arb);
}

TEST(CodecTest, StatusAndWakeup)
{
    Status st;
    st.status = NodeStatus::BusOff;
    std::vector<uint8_t> buf_st = Serializer::serialize(st);
    ASSERT_EQ(buf_st.size(), 9);
    EXPECT_EQ(read_le_u32(buf_st.data()), 0x0041);
    EXPECT_EQ(buf_st[8], 0x03);

    auto ops_st = Deserializer::deserialize_all(buf_st);
    ASSERT_EQ(ops_st.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Status>(ops_st[0]));
    EXPECT_EQ(std::get<Status>(ops_st[0]), st);

    Wakeup wk;
    std::vector<uint8_t> buf_wk = Serializer::serialize(wk);
    ASSERT_EQ(buf_wk.size(), 8);
    EXPECT_EQ(read_le_u32(buf_wk.data()), 0x0042);

    auto ops_wk = Deserializer::deserialize_all(buf_wk);
    ASSERT_EQ(ops_wk.size(), 1);
    ASSERT_TRUE(std::holds_alternative<Wakeup>(ops_wk[0]));
}

TEST(CodecTest, MultiOperationBatching)
{
    std::vector<BusOperation> original_ops;

    CanTransmit tx1;
    tx1.id = 0x10;
    tx1.data = {0x01, 0x02};
    original_ops.push_back(tx1);

    Confirm cf1;
    cf1.id = 0x10;
    original_ops.push_back(cf1);

    CanFdTransmit tx2;
    tx2.id = 0x20;
    tx2.data = {0xAA, 0xBB, 0xCC, 0xDD};
    original_ops.push_back(tx2);

    ArbitrationLost arb1;
    arb1.id = 0x30;
    original_ops.push_back(arb1);

    Wakeup wk1;
    original_ops.push_back(wk1);

    std::vector<uint8_t> batch = Serializer::serialize_batch(original_ops);
    size_t expected_size = tx1.total_length() + cf1.total_length() +
                           tx2.total_length() + arb1.total_length() +
                           wk1.total_length();
    EXPECT_EQ(batch.size(), expected_size);

    std::vector<BusOperation> decoded_ops =
        Deserializer::deserialize_all(batch);
    ASSERT_EQ(decoded_ops.size(), original_ops.size());

    EXPECT_EQ(std::get<CanTransmit>(decoded_ops[0]), tx1);
    EXPECT_EQ(std::get<Confirm>(decoded_ops[1]), cf1);
    EXPECT_EQ(std::get<CanFdTransmit>(decoded_ops[2]), tx2);
    EXPECT_EQ(std::get<ArbitrationLost>(decoded_ops[3]), arb1);
    EXPECT_EQ(std::get<Wakeup>(decoded_ops[4]), wk1);
}

TEST(CodecTest, TruncatedAndCorruptedBuffers)
{
    // 1. Buffer smaller than header (< 8 bytes)
    std::vector<uint8_t> tiny = {0x10, 0x00, 0x00};
    auto res_tiny = Deserializer::deserialize_next(tiny.data(), tiny.size());
    EXPECT_FALSE(res_tiny.success);

    // 2. Truncated operation (Header says length 30, but buffer is only 18
    // bytes)
    std::vector<uint8_t> truncated(18, 0x00);
    write_le_u32(truncated.data(), 0x0010);
    write_le_u32(truncated.data() + 4, 30);
    auto res_trunc =
        Deserializer::deserialize_next(truncated.data(), truncated.size());
    EXPECT_FALSE(res_trunc.success);
    EXPECT_TRUE(std::holds_alternative<FormatError>(res_trunc.operation));

    // 3. Unrecognized opcode
    std::vector<uint8_t> unknown(16, 0x00);
    write_le_u32(unknown.data(), 0x9999);
    write_le_u32(unknown.data() + 4, 16);
    auto res_unknown =
        Deserializer::deserialize_next(unknown.data(), unknown.size());
    EXPECT_FALSE(res_unknown.success);
    EXPECT_TRUE(std::holds_alternative<FormatError>(res_unknown.operation));
}

TEST(CodecTest, NullBufferSafety)
{
    // Calling deserialize_all with nullptr and positive size must return empty
    // without crashing
    auto ops1 = Deserializer::deserialize_all(nullptr, 10);
    EXPECT_TRUE(ops1.empty());

    auto ops2 = Deserializer::deserialize_all(nullptr, 0);
    EXPECT_TRUE(ops2.empty());

    // Calling deserialize_next with nullptr returns failure with FormatError
    auto res = Deserializer::deserialize_next(nullptr, 10);
    EXPECT_FALSE(res.success);
    EXPECT_EQ(res.bytes_consumed, 0);
    EXPECT_TRUE(std::holds_alternative<FormatError>(res.operation));
}

TEST(CodecTest, FormatErrorPopulatedOnLengthViolations)
{
    // CanTransmit header with length < CAN_TRANSMIT_FIXED_SIZE (e.g. length =
    // 12 < 16)
    std::vector<uint8_t> bad_can(12, 0x00);
    write_le_u32(bad_can.data(), static_cast<uint32_t>(OpCode::CanTransmit));
    write_le_u32(bad_can.data() + 4, 12);
    auto res = Deserializer::deserialize_next(bad_can.data(), bad_can.size());
    EXPECT_FALSE(res.success);
    EXPECT_TRUE(std::holds_alternative<FormatError>(res.operation));
    const auto& fe = std::get<FormatError>(res.operation);
    EXPECT_EQ(fe.raw_data.size(), 12);
}
