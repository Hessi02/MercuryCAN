/**
 * \file    main.cpp (tc_14_LoadFromDatabase)
 * \brief   Tests DBC message lookup, boundary values, and signal-size checks.
 *
 * Exercises message lookup against valid, malformed, and boundary DBC rows,
 * including zero- and maximum-length payloads and combinations of signals.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/model/message.hpp>
#include <MercuryCAN/model/database.hpp>

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <typeinfo>

namespace {

constexpr char databaseText[] =
    "VERSION \"\"\n"
    "NS_ :\n"
    "BS_:\n"
    "BU_:\n"
    "BO_ 12 Telemetry: 4 TestNode\n"
    "BO_ 0 Empty: 0 TestNode\n"
    "BO_ 65535 MaximumIdentifier: 8 TestNode\n"
    "BO_ 42 MultipleSignals: 4 TestNode\n"
    "BO_ 65536 IdentifierOverflow: 0 TestNode\n"
    "BO_ 12 PayloadOverflow: 256 TestNode\n"
    "BO_ 12 PayloadTooLarge: 9 TestNode\n"
    "BO_ 4294967296 IdentifierParseOverflow: 0 TestNode\n"
    "BO_ 12 MissingPayload: TestNode\n"
    "BO_ 12: 0 TestNode\n"
    " BO_ 15 IndentedHeader: 0 TestNode\n"
    "BO_ 7 FinalRow: 1 TestNode";
}

int main() {
    constexpr Can::Model::Database database("TestNode", databaseText);
    static_assert(std::is_same_v<
        decltype(database.buildMessage("Empty")),
        Can::Model::Message*
    >);
    static_assert(std::has_virtual_destructor_v<Can::Model::Message>);

    volatile int32_t wheelSpeed;
    auto msg = database.buildMessage("Telemetry", &wheelSpeed);

    assert(msg);
    assert(typeid(Can::Model::Message) == typeid(*msg));
    assert(12 == msg->getIdentifier());
    assert(4 == msg->getPayloadSize());

    //
    // The combined sizes of multiple signal values must exactly match the DBC.
    //
    volatile uint16_t firstSignal = 1;
    volatile uint16_t secondSignal = 2;

    auto multipleSignals = database.buildMessage(
        "MultipleSignals", 
        &firstSignal, 
        &secondSignal
    );

    assert(multipleSignals);
    assert(42 == multipleSignals->getIdentifier());
    assert(4 == multipleSignals->getPayloadSize());

    //
    // Zero-byte and maximum classic-CAN payloads are both valid boundaries.
    //
    auto emptyMessage = database.buildMessage("Empty");

    assert(emptyMessage);
    assert(0 == emptyMessage->getIdentifier());
    assert(0 == emptyMessage->getPayloadSize());

    volatile uint64_t maximumPayload = 0;
    auto maximumMessage = database.buildMessage(
        "MaximumIdentifier", &maximumPayload
    );

    assert(maximumMessage);
    assert(65535 == maximumMessage->getIdentifier());
    assert(8 == maximumMessage->getPayloadSize());

    //
    // A single signal that is larger or smaller than the payload fails.
    //
    volatile uint64_t oversizedSignal;
    auto largeMsg = database.buildMessage("Telemetry", &oversizedSignal);
    assert(!largeMsg);

    volatile int16_t undersizedSignal;
    auto smallMsg = database.buildMessage("Telemetry", &undersizedSignal);
    assert(!smallMsg);

    //
    // The same exact-size rule applies to combinations in either direction.
    //
    auto tooFewSignals = database.buildMessage("MultipleSignals", &firstSignal);
    assert(!tooFewSignals);
    
    volatile uint32_t tooManySignals = 0;
    auto tooMany = database.buildMessage(
        "MultipleSignals", &firstSignal, &tooManySignals
    );

    assert(!tooMany);

    //
    // Lookup is exact and case-sensitive; unknown and null names are rejected.
    //
    assert(!database.buildMessage("Unknown", &wheelSpeed));
    assert(!database.buildMessage("TelemetryExtra", &wheelSpeed));
    assert(!database.buildMessage("telemetry", &wheelSpeed));
    assert(!database.buildMessage(nullptr, &wheelSpeed));

    //
    // Unsupported lengths and malformed/out-of-range header values are rejected.
    //
    assert(!database.buildMessage("IdentifierOverflow"));
    assert(!database.buildMessage("PayloadOverflow"));
    assert(!database.buildMessage("PayloadTooLarge"));
    assert(!database.buildMessage("IdentifierParseOverflow"));
    assert(!database.buildMessage("MissingPayload"));
    assert(!database.buildMessage("IndentedHeader"));

    //
    // A final header without a newline is still scanned.
    //
    volatile uint8_t finalSignal = 0;
    auto finalMessage = database.buildMessage("FinalRow", &finalSignal);

    assert(finalMessage);
    assert(7 == finalMessage->getIdentifier());
    assert(1 == finalMessage->getPayloadSize());

    //
    // A null DBC pointer cannot produce a match.
    //
    constexpr Can::Model::Database nullDatabase("TestNode", nullptr);
    assert(!nullDatabase.buildMessage("Telemetry", &wheelSpeed));

    delete msg;
    delete multipleSignals;
    delete emptyMessage;
    delete maximumMessage;
    delete finalMessage;

    return 0;
}