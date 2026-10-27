/**
 * \file    main.cpp (tc_14_LoadFromDatabase)
 * \brief   Verifies compile-time DBC message construction.
 *
 * Builds messages from both the embedded and an explicit DBC definition and
 * checks identifiers, payload sizes, and cycle times with static assertions.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/model/database.hpp>

#include <cstdint>
#include <type_traits>

namespace {

constexpr Can::Model::Database embeddedDatabase{DATABASE_TEXT};

constexpr char constexprDatabaseText[] =
    "BO_ 12 Telemetry: 4 ECU\n"
    "BO_ 0 Empty: 0 ECU";

constexpr Can::Model::Database constexprDatabase{constexprDatabaseText};

uint32_t embeddedSignal = 0;
uint32_t telemetrySignal = 0;

constexpr Can::Model::Message embeddedMessage =
    embeddedDatabase.buildMessage("Telemetry", &embeddedSignal);
constexpr Can::Model::Message telemetryMessage =
    constexprDatabase.buildMessage("Telemetry", &telemetrySignal);
constexpr Can::Model::Message emptyMessage =
    constexprDatabase.buildMessage("Empty");
constexpr Can::Model::CyclicMessage cyclicMessage =
    constexprDatabase.buildCyclicMessage("Telemetry", 20, &telemetrySignal);
constexpr Can::Model::CyclicMessage emptyCyclicMessage =
    constexprDatabase.buildCyclicMessage("Empty", 30);

static_assert(embeddedMessage.getIdentifier() == 12);
static_assert(embeddedMessage.getPayloadSize() == sizeof(embeddedSignal));
static_assert(telemetryMessage.getIdentifier() == 12);
static_assert(telemetryMessage.getPayloadSize() == sizeof(telemetrySignal));
static_assert(emptyMessage.getIdentifier() == 0);
static_assert(emptyMessage.getPayloadSize() == 0);
static_assert(cyclicMessage.getIdentifier() == 12);
static_assert(cyclicMessage.getPayloadSize() == sizeof(telemetrySignal));
static_assert(cyclicMessage.getCycleTime() == 20);
static_assert(emptyCyclicMessage.getIdentifier() == 0);
static_assert(emptyCyclicMessage.getPayloadSize() == 0);
static_assert(emptyCyclicMessage.getCycleTime() == 30);
static_assert(std::has_virtual_destructor_v<Can::Model::Message>);

}

int main() {
    return 0;
}
