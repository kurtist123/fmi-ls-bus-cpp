/**
 * @file metadata_test.cpp
 * @copyright Copyright (c) 2026 Kurtis Thrush. All rights reserved.
 * @brief Unit tests for FMI 3.0 terminals and layered standard metadata
 * generation
 *
 * For license terms, see the LICENSE file at the root of this project.
 */

#include <gtest/gtest.h>

#include "fmi_ls_bus/metadata/metadata.hpp"

using namespace fmi_ls_bus::metadata;

TEST(MetadataTest, GenerateTerminalsAndIconsBasic)
{
    TerminalConfig cfg;
    cfg.bus_name = "CAN1";
    cfg.tx_data_var = "CAN1.Tx_Data";
    cfg.tx_clock_var = "CAN1.Tx_Clock";
    cfg.rx_data_var = "CAN1.Rx_Data";
    cfg.rx_clock_var = "CAN1.Rx_Clock";
    cfg.include_configuration = false;

    std::string xml = MetadataGenerator::generate_terminals_and_icons(cfg);

    EXPECT_NE(xml.find("<Terminal name=\"CAN1\""), std::string::npos);
    EXPECT_NE(
        xml.find("terminalKind=\"org.fmi-ls-bus.network-terminal\""),
        std::string::npos
    );
    EXPECT_NE(
        xml.find("matchingRule=\"org.fmi-ls-bus.transceiver\""),
        std::string::npos
    );
    EXPECT_NE(
        xml.find(
            "<TerminalMemberVariable memberName=\"Tx_Data\" "
            "variableKind=\"signal\" variableName=\"CAN1.Tx_Data\"/>"
        ),
        std::string::npos
    );
    EXPECT_NE(
        xml.find(
            "<TerminalMemberVariable memberName=\"Tx_Clock\" "
            "variableKind=\"signal\" variableName=\"CAN1.Tx_Clock\"/>"
        ),
        std::string::npos
    );
    EXPECT_NE(
        xml.find(
            "<TerminalMemberVariable memberName=\"Rx_Data\" "
            "variableKind=\"signal\" variableName=\"CAN1.Rx_Data\"/>"
        ),
        std::string::npos
    );
    EXPECT_NE(
        xml.find(
            "<TerminalMemberVariable memberName=\"Rx_Clock\" "
            "variableKind=\"signal\" variableName=\"CAN1.Rx_Clock\"/>"
        ),
        std::string::npos
    );

    // Configuration should not be present
    EXPECT_EQ(xml.find("name=\"Configuration\""), std::string::npos);
}

TEST(MetadataTest, GenerateTerminalsAndIconsWithConfiguration)
{
    TerminalConfig cfg;
    cfg.bus_name = "CAN";
    cfg.include_configuration = true;
    cfg.bus_notifications_var = "CAN.BusNotifications";

    std::string xml = MetadataGenerator::generate_terminals_and_icons(cfg);

    EXPECT_NE(xml.find("<Terminal name=\"Configuration\""), std::string::npos);
    EXPECT_NE(
        xml.find(
            "terminalKind=\"org.fmi-ls-bus.network-terminal.configuration\""
        ),
        std::string::npos
    );
    EXPECT_NE(xml.find("matchingRule=\"bus\""), std::string::npos);
    EXPECT_NE(
        xml.find(
            "<TerminalMemberVariable memberName=\"BusNotifications\" "
            "variableKind=\"signal\" variableName=\"CAN.BusNotifications\"/>"
        ),
        std::string::npos
    );
}

TEST(MetadataTest, GenerateManifest)
{
    std::string manifest_node = MetadataGenerator::generate_manifest(false);
    EXPECT_NE(
        manifest_node.find("<fmiLayeredStandardManifest"),
        std::string::npos
    );
    EXPECT_NE(
        manifest_node.find(
            "fmi-ls:fmi-ls-name=\"org.fmi-standard.fmi-ls-bus\""
        ),
        std::string::npos
    );
    EXPECT_NE(
        manifest_node.find("fmi-ls:fmi-ls-version=\"1.0.0\""),
        std::string::npos
    );
    EXPECT_NE(
        manifest_node.find("isBusSimulationFMU=\"false\""),
        std::string::npos
    );

    std::string manifest_bus = MetadataGenerator::generate_manifest(true);
    EXPECT_NE(
        manifest_bus.find("isBusSimulationFMU=\"true\""),
        std::string::npos
    );
}
