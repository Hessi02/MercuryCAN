/**
 * \file    main.cpp (tc_02_TransmitSingleShot)
 * \brief   Tests the Transmitter::sendMessage() method using mock driver class. 
 *
 * The test firstly initializes a typical Message, a Transmittr and get's the 
 * singleton MockDriver class. Then the sendMessage() method is called. 
 * Afterwards the identifier and payload are checked. If the message is sent 
 * correctly, the test returns successful - output to stderr if not.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <iostream>

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );
   
    Can::Controller::Transmitter tx;

    constexpr uint16_t identifier = 1;
    uint64_t testData = 0x0123456789abcdef;

    if (0 < driver->txTrace.size()) {
        std::cerr << "TX trace not empty before message was sent!" << std::endl;
        return 1;
    }

    for (Can::Driver::MockDriver::FrameEvent& event : driver->txTrace) {
        if (identifier == event.identifier) {
            std::cerr << "Identifier is already in TX trace!" << std::endl;
            return 2;
        }
    }
    
    Can::Model::Message txFrame(
        identifier,
        &testData
    );

    tx.sendMessage(txFrame);

    if (0 == driver->txTrace.size()) {
        std::cerr << "No frame was sent!" << std::endl;
        return 3;
    } else if (1 < driver->txTrace.size()) {
        std::cerr << "More than one frame was sent!" << std::endl;
        return 4;
    }

    for (Can::Driver::MockDriver::FrameEvent& event : driver->txTrace) {
        if (identifier != event.identifier) {
            std::cerr << "Wrong frame identifier was sent!" << std::endl;
            return 5;
        }

        if (sizeof(testData) != event.payload.size()) {
            std::cerr << "Wrong testData size was sent!" << std::endl;
            return 6;
        }

        for (std::size_t byte = 0; byte < sizeof(testData); byte++) {
            const uint8_t expectedByte = static_cast<uint8_t>(
                testData >> ((sizeof(testData) - 1 - byte) * 8)
            );

            if (expectedByte != event.payload[byte]) {
                std::cerr << "Wrong testData byte at index " << byte
                          << "!" << std::endl;
                return 7;
            }
        }
    }

    return 0;
}