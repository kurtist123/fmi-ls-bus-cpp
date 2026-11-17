/**
 * @file test_hex_codec.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Unit tests for hex encoding and decoding
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include <gtest/gtest.h>

#include "fmi_ls_bus/codec/hex_codec.hpp"

using namespace fmi_ls_bus::codec;

TEST(HexCodecTest, EmptyData)
{
    std::vector<uint8_t> empty;
    std::string encoded = HexCodec::encode(empty);
    EXPECT_TRUE(encoded.empty());

    std::vector<uint8_t> decoded;
    EXPECT_TRUE(HexCodec::decode("", decoded));
    EXPECT_TRUE(decoded.empty());

    auto opt = HexCodec::decode("");
    ASSERT_TRUE(opt.has_value());
    EXPECT_TRUE(opt->empty());
}

TEST(HexCodecTest, EncodeBasic)
{
    std::vector<uint8_t> data = {0x00, 0x0F, 0x10, 0xAB, 0xCD, 0xEF};
    std::string lower = HexCodec::encode(data, false);
    EXPECT_EQ(lower, "000f10abcdef");

    std::string upper = HexCodec::encode(data, true);
    EXPECT_EQ(upper, "000F10ABCDEF");
}

TEST(HexCodecTest, DecodeBasicAndMixedCase)
{
    std::vector<uint8_t> decoded;
    EXPECT_TRUE(HexCodec::decode("000f10abcdef", decoded));
    std::vector<uint8_t> expected = {0x00, 0x0F, 0x10, 0xAB, 0xCD, 0xEF};
    EXPECT_EQ(decoded, expected);

    // Upper and mixed case
    std::vector<uint8_t> decoded_mixed;
    EXPECT_TRUE(HexCodec::decode("000F10AbCdEf", decoded_mixed));
    EXPECT_EQ(decoded_mixed, expected);
}

TEST(HexCodecTest, RoundTripBinaryData)
{
    std::vector<uint8_t> binary(256);
    for (size_t i = 0; i < 256; ++i)
    {
        binary[i] = static_cast<uint8_t>(i);
    }

    std::string hex = HexCodec::encode(binary);
    EXPECT_EQ(hex.size(), 512);

    auto decoded = HexCodec::decode(hex);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(*decoded, binary);
}

TEST(HexCodecTest, InvalidHexStrings)
{
    // Odd length
    EXPECT_FALSE(HexCodec::is_valid_hex("1"));
    EXPECT_FALSE(HexCodec::is_valid_hex("123"));
    std::vector<uint8_t> out;
    EXPECT_FALSE(HexCodec::decode("123", out));
    EXPECT_FALSE(HexCodec::decode("123").has_value());

    // Non-hex characters
    EXPECT_FALSE(HexCodec::is_valid_hex("12g4"));
    EXPECT_FALSE(HexCodec::is_valid_hex("0x12"));
    EXPECT_FALSE(HexCodec::decode("12g4", out));
    EXPECT_FALSE(HexCodec::decode("12g4").has_value());
}
