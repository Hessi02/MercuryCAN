/**
 * \file    main.cpp (tc_08_ReceiveMultipleSignals)
 * \brief   Tests applying a received payload to multiple message signals.
 *
 * This testcase checks whether a received message applies the payload to the 
 * contained signals correctly. Each byte needs to point to the correct signal.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <iostream>

int main(int argc, char** argv) {
    Can::Driver::MockDriver* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    constexpr uint16_t identifier = 0x345;
    volatile uint16_t firstSignal = 0;
    volatile uint32_t secondSignal = 0;

    Can::Model::CyclicMessage rxFrame(
        identifier,
        10,
        &firstSignal,
        &secondSignal
    );

    Can::Controller::Receiver rx;
    rx.addCyclicMessage(rxFrame);

    constexpr uint8_t payload[] = {0x12, 0x34, 0x89, 0xab, 0xcd, 0xef};

    driver->injectRxFrame(
        identifier, 
        payload, 
        sizeof(payload)
    );

    if (0x1234 != firstSignal || 0x89abcdef != secondSignal)  {
        std::cerr << "Received payload was not split correctly!" << std::endl;
        return 1;
    }

    if (sizeof(payload) != rxFrame.getPayloadSize()) {
        std::cerr << "Message reported an incorrect payload size!" << std::endl;
        return 2;
    }

    if (!rxFrame.getUpdateFlag()) {
        std::cerr << "Received message did not set update flag!" << std::endl;
        return 3;
    }

    if (1 != driver->rxTrace.size()                               ||
        identifier != driver->rxTrace.front().identifier          ||
        sizeof(payload) != driver->rxTrace.front().payload.size() 
    ) {
        std::cerr << "Mock driver did not record frame correctly!" << std::endl;
        return 4;
    }

    return 0;
}
