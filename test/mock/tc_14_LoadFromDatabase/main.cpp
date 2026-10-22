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
constexpr Can::Model::Database embeddedDatabase{DATABASE_TEXT};
constexpr char constexprDatabaseText[] =
    "BO_ 12 Telemetry: 4 ECU\n"
    "BO_ 0 Empty: 0 ECU";
constexpr Can::Model::Database constexprDatabase{constexprDatabaseText};

static_assert([] {
    int signal = 0;
    auto* message = embeddedDatabase.buildMessage("Telemetry", &signal);
    if (!message)
        return false;

    const bool matches =
        message->getIdentifier() == 12 &&
        message->getPayloadSize() == sizeof(signal);
    delete message;
    return matches;
}());
static_assert([] {
    int signal = 0;
    auto* message = constexprDatabase.buildMessage("Telemetry", &signal);
    if (!message)
        return false;

    const bool matches =
        message->getIdentifier() == 12 &&
        message->getPayloadSize() == sizeof(signal);
    delete message;
    return matches;
}());
static_assert([] {
    auto* message = constexprDatabase.buildCyclicMessage("Empty", 20);
    if (!message)
        return false;

    const bool matches =
        message->getIdentifier() == 0 &&
        message->getPayloadSize() == 0 &&
        message->getCycleTime() == 20;
    delete message;
    return matches;
}());
static_assert(
    constexprDatabase.buildMessage("TelemetryExtra") == nullptr
);
}

int main() {
    const Can::Model::Database database(DATABASE_TEXT);
    
    static_assert(std::is_same_v<
        decltype(database.buildMessage("Empty")),
        Can::Model::Message*
    >);

    static_assert(std::is_same_v<
        decltype(database.buildCyclicMessage("Empty", 1)),
        Can::Model::CyclicMessage*
    >);
    
    static_assert(std::has_virtual_destructor_v<Can::Model::Message>);

    volatile int32_t wheelSpeed;
    auto msg = database.buildMessage("Telemetry", &wheelSpeed);

    assert(msg);
    assert(typeid(Can::Model::Message) == typeid(*msg));
    assert(12 == msg->getIdentifier());
    assert(4 == msg->getPayloadSize());

    auto cyclicMessage = database.buildCyclicMessage(
        "Telemetry", 20, &wheelSpeed
    );

    assert(cyclicMessage);
    assert(12 == cyclicMessage->getIdentifier());
    assert(4 == cyclicMessage->getPayloadSize());
    assert(20 == cyclicMessage->getCycleTime());
    
    volatile int16_t mismatchedSignal = 0;
    
    assert(!database.buildCyclicMessage(
        "Telemetry", 20, &mismatchedSignal
    ));
    
    assert(!database.buildCyclicMessage("Unknown", 20, &wheelSpeed));

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
    constexpr Can::Model::Database nullDatabase(nullptr);
    assert(!nullDatabase.buildMessage("Telemetry", &wheelSpeed));

    delete msg;
    delete cyclicMessage;
    delete multipleSignals;
    delete emptyMessage;
    delete maximumMessage;
    delete finalMessage;

    return 0;
}