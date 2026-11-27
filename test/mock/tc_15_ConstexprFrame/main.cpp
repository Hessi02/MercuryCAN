/**
 * \file    main.cpp (tc_15_ConstexprFrame)
 * \brief   Verifies compile-time construction of Frame and CyclicFrame.
 *
 * Checks zero-, single-, and maximum-signal messages using compile-time
 * assertions for identifiers, payload sizes, and cyclic message periods.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/model/configuration/database.hpp>

#include <cstdint>
#include <string_view>

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
constexpr std::string_view databaseTextView{
    databaseText, sizeof(databaseText) - 1
};
constexpr Can::Model::Configuration::Database database(databaseTextView);
constexpr std::string_view byteMessageName = "ByteMessage";
constexpr std::string_view emptyMessageName = "EmptyMessage";
constexpr std::string_view eightByteMessageName = "EightByteMessage";

constexpr Can::Model::Runtime::Frame emptyFrame(0x100);
constexpr Can::Model::Runtime::Frame singleSignalFrame(0x101, &signal0);
constexpr Can::Model::Runtime::Frame maximumSignalCountFrame(
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
constexpr Can::Model::Runtime::Frame maximumPayloadFrame(0x103, &eightByteSignal);

constexpr Can::Model::Runtime::CyclicFrame emptyCyclicFrame(0x200, 25);
constexpr Can::Model::Runtime::CyclicFrame cyclicFrame(0x201, 50, &signal0);

constexpr Can::Model::Runtime::Frame databaseFrame =
    database.buildMessage(byteMessageName, &signal0);
constexpr Can::Model::Runtime::Frame databaseEmptyFrame =
    database.buildMessage(emptyMessageName);
constexpr Can::Model::Runtime::Frame databaseMaximumPayloadFrame =
    database.buildMessage(eightByteMessageName, &eightByteSignal);
constexpr Can::Model::Runtime::CyclicFrame databaseCyclicFrame =
    database.buildCyclicMessage(byteMessageName, &signal0);
constexpr Can::Model::Runtime::CyclicFrame databaseEmptyCyclicFrame =
    database.buildCyclicMessage(emptyMessageName);
constexpr Can::Model::Runtime::CyclicFrame databaseMaximumPayloadCyclicFrame =
    database.buildCyclicMessage(eightByteMessageName, &eightByteSignal);

static_assert(
    emptyFrame.getIdentifier() == 0x100 &&
    emptyFrame.getPayloadSize() == 0
);
static_assert(
    singleSignalFrame.getIdentifier() == 0x101 &&
    singleSignalFrame.getPayloadSize() == sizeof(signal0)
);
static_assert(
    maximumSignalCountFrame.getIdentifier() == 0x102 &&
    maximumSignalCountFrame.getPayloadSize() == 8
);
static_assert(
    maximumPayloadFrame.getIdentifier() == 0x103 &&
    maximumPayloadFrame.getPayloadSize() == 8
);
static_assert(
    emptyCyclicFrame.getIdentifier() == 0x200 &&
    emptyCyclicFrame.getPayloadSize() == 0 &&
    emptyCyclicFrame.getCycleTime() == 25
);
static_assert(
    cyclicFrame.getIdentifier() == 0x201 &&
    cyclicFrame.getPayloadSize() == sizeof(signal0) &&
    cyclicFrame.getCycleTime() == 50
);
static_assert(
    databaseFrame.getIdentifier() == 512 &&
    databaseFrame.getPayloadSize() == sizeof(signal0)
);
static_assert(
    databaseEmptyFrame.getIdentifier() == 513 &&
    databaseEmptyFrame.getPayloadSize() == 0
);
static_assert(
    databaseMaximumPayloadFrame.getIdentifier() == 514 &&
    databaseMaximumPayloadFrame.getPayloadSize() == 8
);
static_assert(
    databaseCyclicFrame.getIdentifier() == 512 &&
    databaseCyclicFrame.getPayloadSize() == sizeof(signal0) &&
    databaseCyclicFrame.getCycleTime() == 75
);
static_assert(
    databaseEmptyCyclicFrame.getIdentifier() == 513 &&
    databaseEmptyCyclicFrame.getPayloadSize() == 0 &&
    databaseEmptyCyclicFrame.getCycleTime() == 100
);
static_assert(
    databaseMaximumPayloadCyclicFrame.getIdentifier() == 514 &&
    databaseMaximumPayloadCyclicFrame.getPayloadSize() == 8 &&
    databaseMaximumPayloadCyclicFrame.getCycleTime() == 250
);

}

int main() {
    return 0;
}
