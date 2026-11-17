/**
 * @file test_fmu4cpp_adapter.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Unit and integration tests for fmu4cpp FMU model adapter
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include <unordered_map>

#include <gtest/gtest.h>

#include "fmi_ls_bus/adapter/fmu4cpp_adapter.hpp"
#include "fmi_ls_bus/codec/deserializer.hpp"
#include "fmi_ls_bus/codec/serializer.hpp"

#if FMI_LS_BUS_HAS_FMU4CPP

#include <fmu4cpp/fmu_base.hpp>
#include <fmu4cpp/fmu_variable.hpp>

using namespace fmi_ls_bus;
using namespace fmi_ls_bus::adapter;
using namespace fmi_ls_bus::protocol;
using namespace fmi_ls_bus::codec;

namespace
{

// Real FMU Model inheriting from real fmu4cpp::fmu_base using friend binding
class RealFmuModelFriend : public fmu4cpp::fmu_base
{
    FMI_LS_BUS_FMU4CPP_ADAPTER_FRIEND

public:
    Fmu4cppCanAdapter adapter;

    explicit RealFmuModelFriend(
        fmu4cpp::fmu_data data,
        channel::CanChannelConfig ch_cfg = {},
        AdapterConfig ad_cfg = {}
    )
        : fmu_base(std::move(data))
        , adapter(std::move(ch_cfg), std::move(ad_cfg))
    {
        adapter.bind_fmu(*this);
    }

    bool do_step(double) override
    {
        adapter.post_step(*this);
        return true;
    }
};

// Real FMU Model inheriting from real fmu4cpp::fmu_base using explicit variable
// registration
class RealFmuModelCallback : public fmu4cpp::fmu_base
{
public:
    Fmu4cppCanAdapter adapter;

    explicit RealFmuModelCallback(
        fmu4cpp::fmu_data data,
        channel::CanChannelConfig ch_cfg = {},
        AdapterConfig ad_cfg = {}
    )
        : fmu_base(std::move(data))
        , adapter(std::move(ch_cfg), std::move(ad_cfg))
    {
        adapter.register_variables(
            [this](const std::string& name, std::vector<uint8_t>* ptr)
                -> auto& { return this->register_binary(name, ptr); },
            [this](
                const std::string& name,
                fmu4cpp::value_ref vr,
                fmu4cpp::causality_t c
            ) -> auto& { return this->register_clock(name, vr, c); }
        );
    }

    bool do_step(double) override
    {
        adapter.post_step(*this);
        return true;
    }
};

// Real FMU Model using the CRTP / mixin pattern
class RealFmuModelMixin : public Fmu4cppCanModel<RealFmuModelMixin>
{
public:
    explicit RealFmuModelMixin(
        fmu4cpp::fmu_data data,
        channel::CanChannelConfig ch_cfg = {},
        AdapterConfig ad_cfg = {}
    )
        : Fmu4cppCanModel(std::move(data), std::move(ch_cfg), std::move(ad_cfg))
    {
    }

    bool do_step(double) override
    {
        can_adapter_.post_step(*this);
        return true;
    }
};

}  // namespace

namespace fmu4cpp
{
model_info get_model_info()
{
    model_info info;
    info.modelName = "fmu4cpp_can_adapter_test";
    info.hasEventMode = true;
    return info;
}

std::string model_identifier()
{
    return "fmu4cpp_can_adapter_test";
}
}  // namespace fmu4cpp

TEST(Fmu4cppAdapterTest, RealFmuBindingAndVariablesRegistration)
{
    AdapterConfig ad_cfg;
    ad_cfg.bus_name = "Powertrain";
    ad_cfg.max_buffer_size = 4'096;
    ad_cfg.tx_data_vr = 2'001;
    ad_cfg.rx_data_vr = 2'002;
    ad_cfg.tx_clock_vr = 2'003;
    ad_cfg.rx_clock_vr = 2'004;

    channel::CanChannelConfig ch_cfg;
    ch_cfg.name = "Powertrain";
    ch_cfg.max_buffer_size = 4'096;

    fmu4cpp::fmu_data d;
    RealFmuModelFriend fmu(std::move(d), ch_cfg, ad_cfg);

    // Introspect variables on real fmu_base
    auto tx_bin = fmu.get_binary_variable("Powertrain.Tx_Data");
    ASSERT_TRUE(tx_bin.has_value());
    EXPECT_EQ(tx_bin->causality(), fmu4cpp::causality_t::OUTPUT);
    EXPECT_EQ(tx_bin->associated_clock(), 2'003);
    EXPECT_EQ(tx_bin->mime_type(), CAN_MIME_TYPE);
    EXPECT_EQ(tx_bin->max_size(), 4'096);

    auto rx_bin = fmu.get_binary_variable("Powertrain.Rx_Data");
    ASSERT_TRUE(rx_bin.has_value());
    EXPECT_EQ(rx_bin->causality(), fmu4cpp::causality_t::INPUT);
    EXPECT_EQ(rx_bin->associated_clock(), 2'004);
    EXPECT_EQ(rx_bin->mime_type(), CAN_MIME_TYPE);
    EXPECT_EQ(rx_bin->max_size(), 4'096);

    auto tx_clk = fmu.get_clock_variable("Powertrain.Tx_Clock");
    ASSERT_TRUE(tx_clk.has_value());
    EXPECT_EQ(tx_clk->causality(), fmu4cpp::causality_t::OUTPUT);
    EXPECT_EQ(tx_clk->value_reference(), 2'003);

    auto rx_clk = fmu.get_clock_variable("Powertrain.Rx_Clock");
    ASSERT_TRUE(rx_clk.has_value());
    EXPECT_EQ(rx_clk->causality(), fmu4cpp::causality_t::INPUT);
    EXPECT_EQ(rx_clk->value_reference(), 2'004);

    TerminalConfig tc = fmu.adapter.make_terminal_config();
    EXPECT_EQ(tc.bus_name, "Powertrain");
    EXPECT_EQ(tc.tx_data_var, "Powertrain.Tx_Data");
    EXPECT_EQ(tc.tx_clock_var, "Powertrain.Tx_Clock");
    EXPECT_EQ(tc.rx_data_var, "Powertrain.Rx_Data");
    EXPECT_EQ(tc.rx_clock_var, "Powertrain.Rx_Clock");
}

TEST(Fmu4cppAdapterTest, CallbackRegistrationAndLifecycle)
{
    AdapterConfig ad_cfg;
    ad_cfg.bus_name = "CAN";
    ad_cfg.tx_clock_vr = 1'003;
    ad_cfg.rx_clock_vr = 1'004;

    fmu4cpp::fmu_data d;
    RealFmuModelCallback fmu(std::move(d), {}, ad_cfg);

    // Queue frame
    CanTransmit tx;
    tx.id = 0x300;
    tx.data = {0x01, 0x02};
    EXPECT_TRUE(fmu.adapter.channel().queue_transmit(tx));

    // Post step triggers clock and serializes frame into tx_buffer
    EXPECT_TRUE(fmu.step(0.0, 0.01));
    EXPECT_TRUE(fmu.has_pending_output_events());
    EXPECT_TRUE(fmu.get_clock(1'003));
    EXPECT_FALSE(fmu.adapter.tx_buffer().empty());

    // Verify tx_buffer contents
    auto ops = Deserializer::deserialize_all(fmu.adapter.tx_buffer());
    ASSERT_EQ(ops.size(), 1);
    EXPECT_EQ(std::get<CanTransmit>(ops[0]), tx);
}

TEST(Fmu4cppAdapterTest, MixinReceiveLifecycleEventMode)
{
    AdapterConfig ad_cfg;
    ad_cfg.bus_name = "CAN";
    ad_cfg.rx_clock_vr = 1'004;

    fmu4cpp::fmu_data d;
    RealFmuModelMixin fmu(std::move(d), {}, ad_cfg);

    // Create incoming frame
    CanFdTransmit fd;
    fd.id = 0x400;
    fd.data = {0xCA, 0xFE};
    std::vector<uint8_t> payload = Serializer::serialize(fd);

    // Put payload in rx_buffer
    fmu.can_adapter().rx_buffer() = payload;

    // Case 1: Rx clock inactive -> Event mode does not consume
    fmu.set_clock(1'004, false);
    fmu.can_adapter().handle_event_mode(fmu);
    EXPECT_FALSE(fmu.can_adapter().channel().has_pending_rx());
    EXPECT_FALSE(fmu.can_adapter().rx_buffer().empty());

    // Case 2: Rx clock active -> Event mode consumes and clears buffer
    fmu.set_clock(1'004, true);
    fmu.can_adapter().handle_event_mode(fmu);
    EXPECT_TRUE(fmu.can_adapter().channel().has_pending_rx());
    EXPECT_TRUE(fmu.can_adapter().rx_buffer().empty());

    auto op = fmu.can_adapter().channel().pop_rx_operation();
    ASSERT_TRUE(op.has_value());
    EXPECT_EQ(std::get<CanFdTransmit>(*op), fd);
}

#else

TEST(Fmu4cppAdapterTest, StandaloneBuildFallback)
{
    GTEST_SKIP() << "fmu4cpp headers not detected; adapter integration test "
                    "skipped in standalone mode.";
}

#endif
