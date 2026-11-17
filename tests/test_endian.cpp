#include <gtest/gtest.h>

#include "fmi_ls_bus/codec/endian.hpp"
#include "fmi_ls_bus/protocol/constants.hpp"
#include "fmi_ls_bus/protocol/operations.hpp"
#include "fmi_ls_bus/protocol/types.hpp"

using namespace fmi_ls_bus;
using namespace fmi_ls_bus::protocol;
using namespace fmi_ls_bus::codec;

TEST(EndianTest, EndianConversions)
{
    // Test 16-bit
    uint16_t v16 = 0x1234;
    uint8_t buf16[2];
    write_le(buf16, v16);
    EXPECT_EQ(read_le<uint16_t>(buf16), v16);
    EXPECT_EQ(read_le_u16(buf16), v16);

    // Test 32-bit
    uint32_t v32 = 0x12345678;
    uint8_t buf32[4];
    write_le(buf32, v32);
    EXPECT_EQ(read_le<uint32_t>(buf32), v32);
    EXPECT_EQ(read_le_u32(buf32), v32);

    // Test 64-bit
    uint64_t v64 = 0x01234567'89ABCDEFULL;
    uint8_t buf64[8];
    write_le(buf64, v64);
    EXPECT_EQ(read_le<uint64_t>(buf64), v64);
    EXPECT_EQ(read_le_u64(buf64), v64);
}

TEST(ProtocolTypesTest, ConstantsAndMimeTypes)
{
    EXPECT_EQ(
        std::string(CAN_MIME_TYPE),
        "application/org.fmi-standard.fmi-ls-bus.can; version=\"1.0.0\""
    );
    EXPECT_EQ(CAN_MAX_PAYLOAD, 8);
    EXPECT_EQ(CAN_FD_MAX_PAYLOAD, 64);
    EXPECT_EQ(CAN_XL_MAX_PAYLOAD, 2'048);
}

TEST(ProtocolOperationsTest, CanTransmitVariant)
{
    CanTransmit tx;
    tx.id = 0x123;
    tx.data = {0x01, 0x02, 0x03};
    tx.extended_id = false;

    BusOperation op = tx;
    EXPECT_TRUE(std::holds_alternative<CanTransmit>(op));
    const auto& unpacked = std::get<CanTransmit>(op);
    EXPECT_EQ(unpacked.id, 0x123);
    EXPECT_EQ(unpacked.data.size(), 3);
    EXPECT_FALSE(unpacked.extended_id);
    EXPECT_EQ(get_op_code(op), OpCode::CanTransmit);
}
