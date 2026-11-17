#ifndef FMI_LS_BUS_ADAPTER_FMU4CPP_ADAPTER_HPP
#define FMI_LS_BUS_ADAPTER_FMU4CPP_ADAPTER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "fmi_ls_bus/adapter/metadata.hpp"
#include "fmi_ls_bus/channel/can_channel.hpp"
#include "fmi_ls_bus/protocol/constants.hpp"

#if __has_include(<fmu4cpp/fmu_base.hpp>)
#include <fmu4cpp/fmu_base.hpp>
#include <fmu4cpp/fmu_variable.hpp>
#define FMI_LS_BUS_HAS_FMU4CPP 1
#else
#define FMI_LS_BUS_HAS_FMU4CPP 0
#endif

// Macro helper for FMU classes inheriting from fmu4cpp::fmu_base to grant
// binding access
#define FMI_LS_BUS_FMU4CPP_ADAPTER_FRIEND \
    friend class ::fmi_ls_bus::adapter::Fmu4cppCanAdapter;

namespace fmi_ls_bus::adapter
{

struct AdapterConfig
{
    std::string bus_name{"CAN"};
    size_t max_buffer_size{2'048};
    bool bus_notifications{false};
    uint32_t tx_data_vr{1'001};
    uint32_t rx_data_vr{1'002};
    uint32_t tx_clock_vr{1'003};
    uint32_t rx_clock_vr{1'004};
};

class Fmu4cppCanAdapter
{
public:
    explicit Fmu4cppCanAdapter(
        channel::CanChannelConfig channel_config = {},
        AdapterConfig adapter_config = {}
    )
        : channel_(std::move(channel_config))
        , adapter_config_(std::move(adapter_config))
    {
    }

    [[nodiscard]]
    channel::CanChannel& channel() noexcept
    {
        return channel_;
    }

    [[nodiscard]]
    const channel::CanChannel& channel() const noexcept
    {
        return channel_;
    }

    [[nodiscard]]
    const AdapterConfig& config() const noexcept
    {
        return adapter_config_;
    }

    [[nodiscard]]
    std::vector<uint8_t>& tx_buffer() noexcept
    {
        return tx_buffer_;
    }

    [[nodiscard]]
    const std::vector<uint8_t>& tx_buffer() const noexcept
    {
        return tx_buffer_;
    }

    [[nodiscard]]
    std::vector<uint8_t>& rx_buffer() noexcept
    {
        return rx_buffer_;
    }

    [[nodiscard]]
    const std::vector<uint8_t>& rx_buffer() const noexcept
    {
        return rx_buffer_;
    }

    /// Bind adapter variables to an FMU model instance.
    /// Requires that FmuModel grants friendship to Fmu4cppCanAdapter (e.g. via
    /// FMI_LS_BUS_FMU4CPP_ADAPTER_FRIEND) or exposes public registration
    /// methods.
    template <typename FmuModel>
    void bind(FmuModel& fmu)
    {
        bind_fmu(fmu);
    }

    template <typename FmuModel>
    void bind_fmu(FmuModel& fmu)
    {
#if FMI_LS_BUS_HAS_FMU4CPP
        // 1. Register Tx Binary Variable
        std::string tx_data_name = adapter_config_.bus_name + ".Tx_Data";
        auto& tx_var = fmu.register_binary(tx_data_name, &tx_buffer_);
        tx_var.setCausality(fmu4cpp::causality_t::OUTPUT);
        tx_var.setAssociatedClock(adapter_config_.tx_clock_vr);
        tx_var.set_mime_type(std::string(protocol::CAN_MIME_TYPE));
        tx_var.set_max_size(adapter_config_.max_buffer_size);

        // 2. Register Rx Binary Variable
        std::string rx_data_name = adapter_config_.bus_name + ".Rx_Data";
        auto& rx_var = fmu.register_binary(rx_data_name, &rx_buffer_);
        rx_var.setCausality(fmu4cpp::causality_t::INPUT);
        rx_var.setAssociatedClock(adapter_config_.rx_clock_vr);
        rx_var.set_mime_type(std::string(protocol::CAN_MIME_TYPE));
        rx_var.set_max_size(adapter_config_.max_buffer_size);

        // 3. Register Tx Clock (Output Triggered)
        std::string tx_clock_name = adapter_config_.bus_name + ".Tx_Clock";
        fmu.register_clock(
            tx_clock_name,
            adapter_config_.tx_clock_vr,
            fmu4cpp::causality_t::OUTPUT
        );

        // 4. Register Rx Clock (Input Triggered)
        std::string rx_clock_name = adapter_config_.bus_name + ".Rx_Clock";
        fmu.register_clock(
            rx_clock_name,
            adapter_config_.rx_clock_vr,
            fmu4cpp::causality_t::INPUT
        );
#else
        (void)fmu;
#endif
    }

    /// Explicit variable binding via registration functors (requires zero
    /// friendship).
    template <typename BinaryRegFunc, typename ClockRegFunc>
    void register_variables(
        BinaryRegFunc&& reg_binary,
        ClockRegFunc&& reg_clock
    )
    {
#if FMI_LS_BUS_HAS_FMU4CPP
        // 1. Register Tx Binary Variable
        std::string tx_data_name = adapter_config_.bus_name + ".Tx_Data";
        auto& tx_var = reg_binary(tx_data_name, &tx_buffer_);
        tx_var.setCausality(fmu4cpp::causality_t::OUTPUT);
        tx_var.setAssociatedClock(adapter_config_.tx_clock_vr);
        tx_var.set_mime_type(std::string(protocol::CAN_MIME_TYPE));
        tx_var.set_max_size(adapter_config_.max_buffer_size);

        // 2. Register Rx Binary Variable
        std::string rx_data_name = adapter_config_.bus_name + ".Rx_Data";
        auto& rx_var = reg_binary(rx_data_name, &rx_buffer_);
        rx_var.setCausality(fmu4cpp::causality_t::INPUT);
        rx_var.setAssociatedClock(adapter_config_.rx_clock_vr);
        rx_var.set_mime_type(std::string(protocol::CAN_MIME_TYPE));
        rx_var.set_max_size(adapter_config_.max_buffer_size);

        // 3. Register Tx Clock (Output Triggered)
        std::string tx_clock_name = adapter_config_.bus_name + ".Tx_Clock";
        reg_clock(
            tx_clock_name,
            adapter_config_.tx_clock_vr,
            fmu4cpp::causality_t::OUTPUT
        );

        // 4. Register Rx Clock (Input Triggered)
        std::string rx_clock_name = adapter_config_.bus_name + ".Rx_Clock";
        reg_clock(
            rx_clock_name,
            adapter_config_.rx_clock_vr,
            fmu4cpp::causality_t::INPUT
        );
#else
        (void)reg_binary;
        (void)reg_clock;
#endif
    }

    template <typename FmuModel>
    void post_step(FmuModel& fmu)
    {
        if (channel_.has_pending_tx())
        {
            tx_buffer_ = channel_.pack_tx_buffer();
            fmu.notify_output_event(adapter_config_.tx_clock_vr);
        }
        else
        {
            tx_buffer_.clear();
        }
    }

    template <typename FmuModel>
    void handle_event_mode(FmuModel& fmu)
    {
        if (fmu.get_clock(adapter_config_.rx_clock_vr))
        {
            if (!rx_buffer_.empty())
            {
                channel_.unpack_rx_buffer(rx_buffer_);
                rx_buffer_.clear();
            }
        }
    }

    [[nodiscard]]
    TerminalConfig make_terminal_config() const
    {
        TerminalConfig tc;
        tc.bus_name = adapter_config_.bus_name;
        tc.tx_data_var = adapter_config_.bus_name + ".Tx_Data";
        tc.tx_clock_var = adapter_config_.bus_name + ".Tx_Clock";
        tc.rx_data_var = adapter_config_.bus_name + ".Rx_Data";
        tc.rx_clock_var = adapter_config_.bus_name + ".Rx_Clock";
        tc.include_configuration = channel_.notifications_enabled();
        tc.bus_notifications_var =
            adapter_config_.bus_name + ".BusNotifications";
        return tc;
    }

private:
    channel::CanChannel channel_;
    AdapterConfig adapter_config_;
    std::vector<uint8_t> tx_buffer_;
    std::vector<uint8_t> rx_buffer_;
};

#if FMI_LS_BUS_HAS_FMU4CPP
/// Optional CRTP / mixin base class providing an embedded Fmu4cppCanAdapter
/// with automated variable registration in its constructor.
template <typename DerivedModel>
class Fmu4cppCanModel : public fmu4cpp::fmu_base
{
    FMI_LS_BUS_FMU4CPP_ADAPTER_FRIEND

public:
    explicit Fmu4cppCanModel(
        fmu4cpp::fmu_data data,
        channel::CanChannelConfig channel_config = {},
        AdapterConfig adapter_config = {}
    )
        : fmu4cpp::fmu_base(std::move(data))
        , can_adapter_(std::move(channel_config), std::move(adapter_config))
    {
        can_adapter_.bind_fmu(*this);
    }

    [[nodiscard]]
    Fmu4cppCanAdapter& can_adapter() noexcept
    {
        return can_adapter_;
    }

    [[nodiscard]]
    const Fmu4cppCanAdapter& can_adapter() const noexcept
    {
        return can_adapter_;
    }

protected:
    Fmu4cppCanAdapter can_adapter_;
};
#endif

}  // namespace fmi_ls_bus::adapter

#endif  // FMI_LS_BUS_ADAPTER_FMU4CPP_ADAPTER_HPP
