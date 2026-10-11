/**
 * \file    main.cpp (tc_11_TransmitPayloadBoundaries)
 * \brief   Tests empty and maximum-size CAN payload transmission.
 *
 * The test sends a message with no signals and one with the maximum classic
 * CAN payload size, then checks the recorded frame sizes and payload bytes.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/model/message.hpp>

#include <iostream>

bool verifyEmptyFrame(const Can::Driver::MockDriver& driver) {
    constexpr uint16_t emptyIdentifier = 0x400;

    const Can::Driver::MockDriver::FrameEvent& emptyEvent = driver.txTrace[0];

    if (emptyIdentifier != emptyEvent.identifier ||
        0 != emptyEvent.payload.size()
    ) {
        std::cerr << "Empty data was not transmitted correctly!" << std::endl;
        return false;
    }

    return true;
}

bool verifyMaximumFrame(const Can::Driver::MockDriver& driver) {
    constexpr uint16_t maximumIdentifier = 0x401;
    constexpr uint64_t expectedPayload = 0x0123456789abcdef;

    const Can::Driver::MockDriver::FrameEvent& maximumEvent = driver.txTrace[1];
    
    if (maximumIdentifier != maximumEvent.identifier ||
        8 != maximumEvent.payload.size()
    ) {
        std::cerr << "Maximum data was not transmitted correctly!" << std::endl;
        return false;
    }

    for (std::size_t index = 0; index < maximumEvent.payload.size(); index++) {
        const uint8_t expectedByte = static_cast<uint8_t>(
            expectedPayload >> ((7 - index) * 8)
        );

        if (expectedByte != maximumEvent.payload[index]) {
            std::cerr << "Maximum data has an incorrect byte!" << std::endl;
            return false;
        }
    }

    return true;
}

bool verifyFrameCount(const Can::Driver::MockDriver& driver) {
    if (2 == driver.txTrace.size())
        return true;

    std::cerr << "Boundary test did not record two frames!" << std::endl;
    return false;
}

int main() {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    Can::Controller::Transmitter tx;
    driver->stopTxTimer();

    constexpr uint16_t emptyIdentifier   = 0x400;
    constexpr uint16_t maximumIdentifier = 0x401;
    constexpr uint64_t maximumPayload    = 0x0123456789abcdef;

    volatile uint64_t payload = maximumPayload;

    Can::Model::Message emptyFrame(emptyIdentifier);
    Can::Model::Message maximumFrame(maximumIdentifier, &payload);
    
    tx.sendMessage(emptyFrame);
    tx.sendMessage(maximumFrame);

    if (!verifyFrameCount(*driver))
        return 1;

    if (!verifyEmptyFrame(*driver) || !verifyMaximumFrame(*driver))
        return 2;

    return 0;
}
