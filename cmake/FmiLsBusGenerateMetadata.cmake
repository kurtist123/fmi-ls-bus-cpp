############################################################
# @file FmiLsBusGenerateMetadata.cmake
# @brief CMake helper to generate FMI LS-BUS metadata conforming to FMI 3.0
#
# Copyright (c) 2026 Kurtis Thrush. All rights reserved.
# For license terms, see the LICENSE file at the root of this project.
############################################################

#[=======================================================================[.rst:
fmi_ls_bus_generate_metadata
----------------------------

Generate FMI LS-BUS Layered Standard metadata files conforming to the
FMI LS-BUS 1.0.0 specification and FMI 3.0 Terminals and Icons schema.

Creates:
  - ``<OUTPUT_DIRECTORY>/terminalsAndIcons/terminalsAndIcons.xml``
  - ``<OUTPUT_DIRECTORY>/extra/org.fmi-standard.fmi-ls-bus/fmi-ls-manifest.xml``

Usage:
  fmi_ls_bus_generate_metadata(
      [OUTPUT_DIRECTORY <dir>]
      BUSES <bus1> [<bus2> ...]
      [IS_BUS_SIMULATION]
      [DESCRIPTION <text>]
      [TERMINALS_FOLDER_VAR <var>]
      [EXTRA_FOLDER_VAR <var>]
  )

Arguments:
  ``OUTPUT_DIRECTORY <dir>``
    Base output directory where metadata will be created.
    Defaults to ``${CMAKE_CURRENT_BINARY_DIR}/ls_bus_metadata``.

  ``BUSES <bus1> ...``
    One or more CAN bus identifiers (e.g., ``CAN1``, ``CAN2``, ``CAN3``).
    For each bus ``<B>``, the generator creates a network terminal declaring
    ``<B>.Tx_Data``, ``<B>.Tx_Clock``, ``<B>.Rx_Data``, ``<B>.Rx_Clock``,
    and a nested Configuration terminal for ``<B>.BusNotifications``.

  ``IS_BUS_SIMULATION``
    Optional flag indicating whether the FMU acts as a bus simulation FMU
    (default: false).

  ``DESCRIPTION <text>``
    Human-readable description written into ``fmi-ls-manifest.xml``.
    Defaults to "Layered Standard for the simulation of bus communication".

  ``TERMINALS_FOLDER_VAR <var>``
    Variable in parent scope to receive the path to the terminalsAndIcons folder,
    suitable for passing to ``generateFMU(TERMINALS_AND_ICONS_FOLDER ...)``.

  ``EXTRA_FOLDER_VAR <var>``
    Variable in parent scope to receive the path to the extra folder,
    suitable for passing to ``generateFMU(EXTRA_FOLDER ...)``.
#]=======================================================================]
function(fmi_ls_bus_generate_metadata)
    set(options
        IS_BUS_SIMULATION
    )
    set(oneValueArgs
        OUTPUT_DIRECTORY
        DESCRIPTION
        TERMINALS_FOLDER_VAR
        EXTRA_FOLDER_VAR
    )
    set(multiValueArgs
        BUSES
    )

    cmake_parse_arguments(PARSE_ARGV 0 ARG "${options}" "${oneValueArgs}" "${multiValueArgs}")

    if(NOT ARG_BUSES)
        message(FATAL_ERROR "fmi_ls_bus_generate_metadata: BUSES argument must contain at least one bus name")
    endif()

    if(NOT ARG_OUTPUT_DIRECTORY)
        set(ARG_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ls_bus_metadata")
    endif()

    if(NOT ARG_DESCRIPTION)
        set(ARG_DESCRIPTION "Layered Standard for the simulation of bus communication")
    endif()

    set(terminals_dir "${ARG_OUTPUT_DIRECTORY}/terminalsAndIcons")
    set(extra_dir "${ARG_OUTPUT_DIRECTORY}/extra/org.fmi-standard.fmi-ls-bus")

    file(MAKE_DIRECTORY "${terminals_dir}")
    file(MAKE_DIRECTORY "${extra_dir}")

    # Build terminalsAndIcons.xml
    set(terminals_xml "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    string(APPEND terminals_xml "<fmiTerminalsAndIcons xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" fmiVersion=\"3.0\">\n")
    string(APPEND terminals_xml "    <Terminals>\n")

    foreach(bus IN LISTS ARG_BUSES)
        string(APPEND terminals_xml "        <Terminal name=\"${bus}\"\n")
        string(APPEND terminals_xml "                  terminalKind=\"org.fmi-ls-bus.network-terminal\"\n")
        string(APPEND terminals_xml "                  matchingRule=\"org.fmi-ls-bus.transceiver\">\n")
        string(APPEND terminals_xml "            <TerminalMemberVariable memberName=\"Tx_Data\" variableKind=\"signal\" variableName=\"${bus}.Tx_Data\"/>\n")
        string(APPEND terminals_xml "            <TerminalMemberVariable memberName=\"Tx_Clock\" variableKind=\"signal\" variableName=\"${bus}.Tx_Clock\"/>\n")
        string(APPEND terminals_xml "            <TerminalMemberVariable memberName=\"Rx_Data\" variableKind=\"signal\" variableName=\"${bus}.Rx_Data\"/>\n")
        string(APPEND terminals_xml "            <TerminalMemberVariable memberName=\"Rx_Clock\" variableKind=\"signal\" variableName=\"${bus}.Rx_Clock\"/>\n")
        string(APPEND terminals_xml "            <Terminal name=\"Configuration\"\n")
        string(APPEND terminals_xml "                      terminalKind=\"org.fmi-ls-bus.network-terminal.configuration\"\n")
        string(APPEND terminals_xml "                      matchingRule=\"bus\">\n")
        string(APPEND terminals_xml "                <TerminalMemberVariable memberName=\"BusNotifications\" variableKind=\"signal\" variableName=\"${bus}.BusNotifications\"/>\n")
        string(APPEND terminals_xml "            </Terminal>\n")
        string(APPEND terminals_xml "        </Terminal>\n")
    endforeach()

    string(APPEND terminals_xml "    </Terminals>\n")
    string(APPEND terminals_xml "</fmiTerminalsAndIcons>\n")

    file(WRITE "${terminals_dir}/terminalsAndIcons.xml" "${terminals_xml}")

    # Build fmi-ls-manifest.xml
    set(is_bus_sim "false")
    if(ARG_IS_BUS_SIMULATION)
        set(is_bus_sim "true")
    endif()

    set(manifest_xml "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    string(APPEND manifest_xml "<fmiLayeredStandardManifest\n")
    string(APPEND manifest_xml "    xmlns:fmi-ls=\"http://fmi-standard.org/fmi-ls-manifest\"\n")
    string(APPEND manifest_xml "    fmi-ls:fmi-ls-name=\"org.fmi-standard.fmi-ls-bus\"\n")
    string(APPEND manifest_xml "    fmi-ls:fmi-ls-version=\"1.0.0\"\n")
    string(APPEND manifest_xml "    fmi-ls:fmi-ls-description=\"${ARG_DESCRIPTION}\"\n")
    string(APPEND manifest_xml "    isBusSimulationFMU=\"${is_bus_sim}\"/>\n")

    file(WRITE "${extra_dir}/fmi-ls-manifest.xml" "${manifest_xml}")

    if(ARG_TERMINALS_FOLDER_VAR)
        set("${ARG_TERMINALS_FOLDER_VAR}" "${terminals_dir}" PARENT_SCOPE)
    endif()

    if(ARG_EXTRA_FOLDER_VAR)
        set("${ARG_EXTRA_FOLDER_VAR}" "${ARG_OUTPUT_DIRECTORY}/extra" PARENT_SCOPE)
    endif()
endfunction()
