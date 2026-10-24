/**
 * \file    main.cpp (tc_05_TransmitMultipleSignals)
 * \brief   Tests serialization and transmission of multiple message signals.
 *
 * This testcase checks whether multiple signals in a frame are transmitted 
 * correctly. The test checks for the correct identifier and payload. If an
 * error occurrs, the information is printed to stderr.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>
#include <MercuryCAN/model/message.hpp>

#include <array>
#include <iostream>

namespace {

static_assert([] {
    int signal = 0;
    Can::Model::Message message{0x123, &signal};
    return message.getIdentifier() == 0x123 &&
           message.getPayloadSize() == sizeof(signal);
}());
static_assert([] {
    Can::Model::Message message{0x124};
    return message.getIdentifier() == 0x124 &&
           message.getPayloadSize() == 0;
}());
static_assert([] {
    int signal = 0;
    Can::Model::CyclicMessage message{0x125, 10, &signal};
    return message.getIdentifier() == 0x125 &&
           message.getCycleTime() == 10 &&
           message.getPayloadSize() == sizeof(signal);
}());
static_assert([] {
    Can::Model::CyclicMessage message{0x126, 20};
    return message.getIdentifier() == 0x126 &&
           message.getCycleTime() == 20 &&
           message.getPayloadSize() == 0;
}());

}

int main(int argc, char** argv) {
    Can::Driver::MockDriver* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    Can::Controller::Transmitter tx;

    constexpr uint16_t identifier  = 0x321;
    volatile uint16_t firstSignal  = 0x1234;
    volatile uint32_t secondSignal = 0x89abcdef;

    Can::Model::Message frame(
        identifier, 
        &firstSignal, 
        &secondSignal
    );

    tx.sendMessage(frame);

    constexpr std::array<uint8_t, 6> expectedPayload{
        0x12, 0x34, 0x89, 0xab, 0xcd, 0xef
    };

    if (1 != driver->txTrace.size()) {
        std::cerr << "Expected exactly one transmitted frame!" << std::endl;
        return 1;
    }

    const Can::Driver::MockDriver::FrameEvent& event = driver->txTrace.front();

    if (identifier != event.identifier) {
        std::cerr << "Transmitted frame has the wrong identifier!" << std::endl;
        return 2;
    }

    if (event.payload.size() != expectedPayload.size()) {
        std::cerr << "Transmitted payload has the wrong size!" << std::endl;
        return 3;
    }

    for (std::size_t index = 0; index < expectedPayload.size(); index++) {
        if (event.payload[index] != expectedPayload[index]) {
            std::cerr << "Transmitted payload has incorrect data!" << std::endl;
            return 4;
        }
    }

    return 0;
}
