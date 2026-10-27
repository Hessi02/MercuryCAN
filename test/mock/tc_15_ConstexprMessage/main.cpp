/**
 * \file    main.cpp (tc_15_ConstexprMessage)
 * \brief   Verifies compile-time construction of Message and CyclicMessage.
 *
 * Checks zero-, single-, and maximum-signal messages using compile-time
 * assertions for identifiers, payload sizes, and cyclic message periods.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/model/database.hpp>

#include <cstdint>

namespace {

uint8_t signal0 = 0;
uint8_t signal1 = 0;
uint8_t signal2 = 0;
uint8_t signal3 = 0;
uint8_t signal4 = 0;
uint8_t signal5 = 0;
uint8_t signal6 = 0;
uint8_t signal7 = 0;
uint64_t eightByteSignal = 0;

constexpr char databaseText[] =
    "BO_ 512 ByteMessage: 1 ECU\n"
    "BO_ 513 EmptyMessage: 0 ECU\n"
    "BO_ 514 EightByteMessage: 8 ECU\n"
    "BA_ \"GenMsgSendType\" BO_ 512 \"cyclicIfActive\";\n"
    "BA_ \"GenMsgCycleTime\" BO_ 512 75;\n"
    "BA_ \"GenMsgSendType\" BO_ 513 \"cyclicAndTriggered\";\n"
    "BA_ \"GenMsgCycleTime\" BO_ 513 100;\n"
    "BA_ \"GenMsgSendType\" BO_ 514 \"cyclicIfActiveAndTriggered\";\n"
    "BA_ \"GenMsgCycleTime\" BO_ 514 250;";
constexpr Can::Model::Database database(databaseText);

constexpr Can::Model::Message emptyMessage(0x100);
constexpr Can::Model::Message singleSignalMessage(0x101, &signal0);
constexpr Can::Model::Message maximumSignalCountMessage(
    0x102,
    &signal0,
    &signal1,
    &signal2,
    &signal3,
    &signal4,
    &signal5,
    &signal6,
    &signal7
);
constexpr Can::Model::Message maximumPayloadMessage(0x103, &eightByteSignal);

constexpr Can::Model::CyclicMessage emptyCyclicMessage(0x200, 25);
constexpr Can::Model::CyclicMessage cyclicMessage(0x201, 50, &signal0);

constexpr Can::Model::Message databaseMessage =
    database.buildMessage("ByteMessage", &signal0);
constexpr Can::Model::Message databaseEmptyMessage =
    database.buildMessage("EmptyMessage");
constexpr Can::Model::Message databaseMaximumPayloadMessage =
    database.buildMessage("EightByteMessage", &eightByteSignal);
constexpr Can::Model::CyclicMessage databaseCyclicMessage =
    database.buildCyclicMessage("ByteMessage", &signal0);
constexpr Can::Model::CyclicMessage databaseEmptyCyclicMessage =
    database.buildCyclicMessage("EmptyMessage");
constexpr Can::Model::CyclicMessage databaseMaximumPayloadCyclicMessage =
    database.buildCyclicMessage("EightByteMessage", &eightByteSignal);

static_assert(
    emptyMessage.getIdentifier() == 0x100 &&
    emptyMessage.getPayloadSize() == 0
);
static_assert(
    singleSignalMessage.getIdentifier() == 0x101 &&
    singleSignalMessage.getPayloadSize() == sizeof(signal0)
);
static_assert(
    maximumSignalCountMessage.getIdentifier() == 0x102 &&
    maximumSignalCountMessage.getPayloadSize() == 8
);
static_assert(
    maximumPayloadMessage.getIdentifier() == 0x103 &&
    maximumPayloadMessage.getPayloadSize() == 8
);
static_assert(
    emptyCyclicMessage.getIdentifier() == 0x200 &&
    emptyCyclicMessage.getPayloadSize() == 0 &&
    emptyCyclicMessage.getCycleTime() == 25
);
static_assert(
    cyclicMessage.getIdentifier() == 0x201 &&
    cyclicMessage.getPayloadSize() == sizeof(signal0) &&
    cyclicMessage.getCycleTime() == 50
);
static_assert(
    databaseMessage.getIdentifier() == 512 &&
    databaseMessage.getPayloadSize() == sizeof(signal0)
);
static_assert(
    databaseEmptyMessage.getIdentifier() == 513 &&
    databaseEmptyMessage.getPayloadSize() == 0
);
static_assert(
    databaseMaximumPayloadMessage.getIdentifier() == 514 &&
    databaseMaximumPayloadMessage.getPayloadSize() == 8
);
static_assert(
    databaseCyclicMessage.getIdentifier() == 512 &&
    databaseCyclicMessage.getPayloadSize() == sizeof(signal0) &&
    databaseCyclicMessage.getCycleTime() == 75
);
static_assert(
    databaseEmptyCyclicMessage.getIdentifier() == 513 &&
    databaseEmptyCyclicMessage.getPayloadSize() == 0 &&
    databaseEmptyCyclicMessage.getCycleTime() == 100
);
static_assert(
    databaseMaximumPayloadCyclicMessage.getIdentifier() == 514 &&
    databaseMaximumPayloadCyclicMessage.getPayloadSize() == 8 &&
    databaseMaximumPayloadCyclicMessage.getCycleTime() == 250
);

}

int main() {
    return 0;
}
