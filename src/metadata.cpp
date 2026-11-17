#include "fmi_ls_bus/adapter/metadata.hpp"

#include <sstream>

namespace fmi_ls_bus::adapter
{

std::string MetadataGenerator::generate_terminals_and_icons(
    const TerminalConfig& config
)
{
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << "<fmiTerminalsAndIcons "
           "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n";
    oss << "                      fmiVersion=\"3.0\">\n";
    oss << "    <Terminals>\n";
    oss << "        <Terminal name=\"" << config.bus_name << "\"\n";
    oss << "                  "
           "terminalKind=\"org.fmi-ls-bus.network-terminal\"\n";
    oss << "                  "
           "matchingRule=\"org.fmi-ls-bus.transceiver\">\n";
    oss << "            <TerminalMemberVariable memberName=\"Tx_Data\" "
           "variableKind=\"signal\" variableName=\""
        << config.tx_data_var << "\"/>\n";
    oss << "            <TerminalMemberVariable memberName=\"Tx_Clock\" "
           "variableKind=\"signal\" variableName=\""
        << config.tx_clock_var << "\"/>\n";
    oss << "            <TerminalMemberVariable memberName=\"Rx_Data\" "
           "variableKind=\"signal\" variableName=\""
        << config.rx_data_var << "\"/>\n";
    oss << "            <TerminalMemberVariable memberName=\"Rx_Clock\" "
           "variableKind=\"signal\" variableName=\""
        << config.rx_clock_var << "\"/>\n";

    if (config.include_configuration)
    {
        oss << "            <Terminal name=\"Configuration\"\n";
        oss << "                      "
               "terminalKind=\"org.fmi-ls-bus.network-terminal.configuration\""
               "\n";
        oss << "                      matchingRule=\"bus\">\n";
        oss << "                <TerminalMemberVariable "
               "memberName=\"BusNotifications\" variableKind=\"signal\" "
               "variableName=\""
            << config.bus_notifications_var << "\"/>\n";
        oss << "            </Terminal>\n";
    }

    oss << "        </Terminal>\n";
    oss << "    </Terminals>\n";
    oss << "</fmiTerminalsAndIcons>\n";

    return oss.str();
}

std::string MetadataGenerator::generate_manifest(
    bool is_bus_simulation,
    std::string_view description
)
{
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << "<fmiLayeredStandardManifest\n";
    oss << "    xmlns:fmi-ls=\"http://fmi-standard.org/fmi-ls-manifest\"\n";
    oss << "    fmi-ls:fmi-ls-name=\"org.fmi-standard.fmi-ls-bus\"\n";
    oss << "    fmi-ls:fmi-ls-version=\"1.0.0\"\n";
    oss << "    fmi-ls:fmi-ls-description=\"" << description << "\"\n";
    oss << "    isBusSimulationFMU=\"" << (is_bus_simulation ? "true" : "false")
        << "\"/>\n";

    return oss.str();
}

}  // namespace fmi_ls_bus::adapter
