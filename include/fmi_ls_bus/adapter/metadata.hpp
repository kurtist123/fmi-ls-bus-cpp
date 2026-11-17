/**
 * @file metadata.hpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Generators for FMI 3.0 terminalsAndIcons.xml and fmi-ls-manifest.xml
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#ifndef FMI_LS_BUS_ADAPTER_METADATA_HPP
#define FMI_LS_BUS_ADAPTER_METADATA_HPP

#include <string>
#include <string_view>

namespace fmi_ls_bus::adapter
{

struct TerminalConfig
{
    std::string bus_name{"CAN"};
    std::string tx_data_var{"CAN.Tx_Data"};
    std::string tx_clock_var{"CAN.Tx_Clock"};
    std::string rx_data_var{"CAN.Rx_Data"};
    std::string rx_clock_var{"CAN.Rx_Clock"};
    bool include_configuration{false};
    std::string bus_notifications_var{"CAN.BusNotifications"};
};

class MetadataGenerator
{
public:
    static std::string generate_terminals_and_icons(
        const TerminalConfig& config
    );

    static std::string generate_manifest(
        bool is_bus_simulation = false,
        std::string_view description =
            "Layered Standard for the simulation of bus communication on a "
            "Physical Signal Abstraction or Network Abstraction based level."
    );
};

}  // namespace fmi_ls_bus::adapter

#endif  // FMI_LS_BUS_ADAPTER_METADATA_HPP
