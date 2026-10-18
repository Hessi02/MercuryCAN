/**
 * \file    main.cpp (tc_14_LoadFromDatabase)
 * \brief   Tests message construction from an in-memory DBC definition.
 *
 * Provides a minimal DBC string containing one message and constructs a
 * Database over that text. The test verifies that the matching message's
 * identifier and declared payload length are used, and that construction
 * fails cleanly when the message name is unknown or the signal data exceeds
 * the DBC-declared payload length.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/model/message.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>
#include <MercuryCAN/model/database.hpp>

#include <iostream>
#include <cassert>

int main(int argc, char** argv) {
    constexpr Can::Model::Database database(
        "TestNode",
        "VERSION \"\"\n"
        "\n"
        "\n"
        "NS_ : \n"
        "\t\tNS_DESC_\n"
        "\t\tCM_\n"
        "\t\tBA_DEF_\n"
        "\t\tBA_\n"
        "\t\tVAL_\n"
        "\t\tCAT_DEF_\n"
        "\t\tCAT_\n"
        "\t\tFILTER\n"
        "\t\tBA_DEF_DEF_\n"
        "\t\tEV_DATA_\n"
        "\t\tENVVAR_DATA_\n"
        "\t\tSGTYPE_\n"
        "\t\tSGTYPE_VAL_\n"
        "\t\tBA_DEF_SGTYPE_\n"
        "\t\tBA_SGTYPE_\n"
        "\t\tSIG_TYPE_REF_\n"
        "\t\tVAL_TABLE_\n"
        "\t\tSIG_GROUP_\n"
        "\t\tSIG_VALTYPE_\n"
        "\t\tSIGTYPE_VALTYPE_\n"
        "\t\tBO_TX_BU_\n"
        "\t\tBA_DEF_REL_\n"
        "\t\tBA_REL_\n"
        "\t\tBA_DEF_DEF_REL_\n"
        "\t\tBU_SG_REL_\n"
        "\t\tBU_EV_REL_\n"
        "\t\tBU_BO_REL_\n"
        "\t\tSG_MUL_VAL_\n"
        "\n"
        "BS_:\n"
        "\n"
        "BU_:\n"
        "\n"
        "\n"
        "\n"
        "BO_ 12 Telemetry: 4 TestNode\n"
        "\tSG_ WheelSpeed : 0|32@1- (1,0) [-1500|1500] \"rpm\" Vector__XXX\n"
        "\n"
        "\n"
        "CM_ BO_ 1 \"Vehicle telemetry data\";\n"
        "CM_ SG_ 1 WheelSpeed \"Wheel speed sensor data.\";\n"
        "BA_DEF_ BO_  \"VFrameFormat\" ENUM  \"StandardCAN\",\"ExtendedCAN\",\"reserved\",\"J1939PG\";\n"
        "BA_DEF_  \"DatabaseVersion\" STRING ;\n"
        "BA_DEF_  \"BusType\" STRING ;\n"
        "BA_DEF_  \"ProtocolType\" STRING ;\n"
        "BA_DEF_DEF_  \"VFrameFormat\" \"StandardCAN\";\n"
        "BA_DEF_DEF_  \"DatabaseVersion\" \"1.0.0\";\n"
        "BA_DEF_DEF_  \"BusType\" \"CAN\";\n"
        "BA_DEF_DEF_  \"ProtocolType\" \"\";\n"
        "BA_ \"DatabaseVersion\" \"1.0.0\";\n"
        "BA_ \"BusType\" \"CAN\";\n"
        "BA_ \"ProtocolType\" \"\";"
    );

    volatile int32_t wheelSpeed;
    auto msg = database.buildMessage("Telemetry", &wheelSpeed);

    //
    // 1. Check whether the parsing algortihm finds the message in DBC.
    //
    assert(msg && "Message could be found in DBC.");

    //
    // 2. Check whether the parsing algorithm does not include a cycle time.
    //
    assert(typeid(Can::Model::Message) == typeid(*msg) && "Single shot message was build.");

    //
    // 3. Check if the identifier can be read from DBC.
    //
    assert(0x0c == msg->getIdentifier() && "Identifier gets parsed from DBC.");

    //
    // 4. Check if the payload size can be parsed from DBC.
    //
    assert(0x04 == msg->getPayloadSize() && "Payload size gets parsed from DBC.");

    //
    // 5. Check if an oversized signal results in an unpopulated variable.
    //
    volatile uint64_t oversizedSignal;
    auto largeMsg = database.buildMessage("Telemetry", &oversizedSignal);
    assert(!largeMsg && "Invalid signal size results in invalid message.");

    //
    // 6. Check if an undersized signal results in an upopulated variable.
    //
    volatile int16_t undersizedSignal;
    auto smallMsg = database.buildMessage("Telemetry", &undersizedSignal);
    assert(!smallMsg && "Invalid signal size results in invalid message.");
    
    //
    // 7. Check if an unknown signal results in an unpopulated variable.
    //
    auto unknownMsg = database.buildMessage("Unknown", &wheelSpeed); 
    assert(!unknownMsg && "Unknown signal results in invalid message.");

    return 0;
}