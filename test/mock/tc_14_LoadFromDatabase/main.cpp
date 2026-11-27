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

#include <MercuryCAN/model/configuration/database.hpp>

#include <cstdint>
#include <type_traits>

namespace {

constexpr Can::Model::Configuration::Database embeddedDatabase{DATABASE_TEXT};

constexpr char constexprDatabaseText[] =
    "BO_ 12 Telemetry: 4 ECU\n"
    "BO_ 0 Empty: 0 ECU\n"
    "BA_ \"GenMsgSendType\" BO_ 12 \"cyclic\";\n"
    "BA_ \"GenMsgCycleTime\" BO_ 12 20;\n"
    "BA_ \"GenMsgSendType\" BO_ 0 \"cyclicAndTriggered\";\n"
    "BA_ \"GenMsgCycleTime\" BO_ 0 30;";

constexpr Can::Model::Configuration::Database constexprDatabase{constexprDatabaseText};

uint32_t embeddedSignal = 0;
uint32_t telemetrySignal = 0;

constexpr Can::Model::Runtime::Frame embeddedFrame =
    embeddedDatabase.buildMessage("Telemetry", &embeddedSignal);
constexpr Can::Model::Runtime::Frame telemetryFrame =
    constexprDatabase.buildMessage("Telemetry", &telemetrySignal);
constexpr Can::Model::Runtime::Frame emptyFrame =
    constexprDatabase.buildMessage("Empty");
constexpr Can::Model::Runtime::CyclicFrame cyclicFrame =
    constexprDatabase.buildCyclicMessage("Telemetry", &telemetrySignal);
constexpr Can::Model::Runtime::CyclicFrame emptyCyclicFrame =
    constexprDatabase.buildCyclicMessage("Empty");

static_assert(embeddedFrame.getIdentifier() == 12);
static_assert(embeddedFrame.getPayloadSize() == sizeof(embeddedSignal));
static_assert(telemetryFrame.getIdentifier() == 12);
static_assert(telemetryFrame.getPayloadSize() == sizeof(telemetrySignal));
static_assert(emptyFrame.getIdentifier() == 0);
static_assert(emptyFrame.getPayloadSize() == 0);
static_assert(cyclicFrame.getIdentifier() == 12);
static_assert(cyclicFrame.getPayloadSize() == sizeof(telemetrySignal));
static_assert(cyclicFrame.getCycleTime() == 20);
static_assert(emptyCyclicFrame.getIdentifier() == 0);
static_assert(emptyCyclicFrame.getPayloadSize() == 0);
static_assert(emptyCyclicFrame.getCycleTime() == 30);
static_assert(std::has_virtual_destructor_v<Can::Model::Runtime::Frame>);

}

int main() {
    return 0;
}
