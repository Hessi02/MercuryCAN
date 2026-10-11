/**
 * \file    main.cpp (tc_10_ReceiveShortPayload)
 * \brief   Tests reception of a payload shorter than its configured signals.
 *
 * The test verifies that complete leading signals are updated while a signal
 * that does not fit in the received payload retains its previous value.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <iostream>

bool verifySignals(
    const volatile uint8_t& firstSignal,
    const volatile uint16_t& secondSignal
) {
    constexpr uint8_t expectedFirstSignal   = 0xa5;
    constexpr uint16_t expectedSecondSignal = 0x1234;

    if (expectedFirstSignal != firstSignal   ||
        expectedSecondSignal != secondSignal
    ) {
        std::cerr << "Did not update only complete signals!" << std::endl;
        return false;
    }

    return true;
}

bool verifyRecordedPayload(const Can::Driver::MockDriver& driver) {
    if (1 != driver.rxTrace.size()                 ||
        1 != driver.rxTrace.front().payload.size()
    ) {
        std::cerr << "Did not record short payload correctly!" << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    constexpr uint16_t identifier = 0x321;
    constexpr uint8_t receivedByte = 0xa5;

    volatile uint8_t firstSignal = 0;
    volatile uint16_t secondSignal = 0x1234;

    Can::Model::CyclicMessage rxFrame(
        identifier, 
        10, 
        &firstSignal, 
        &secondSignal
    );

    Can::Controller::Receiver rx;
    rx.addCyclicMessage(rxFrame);

    driver->injectRxFrame(
        identifier, 
        &receivedByte, 
        sizeof(receivedByte)
    );

    if (!verifySignals(firstSignal, secondSignal))
        return 1;

    if (!rxFrame.getUpdateFlag()) {
        std::cerr << "Matching payload did not set update flag!" << std::endl;
        return 2;
    }

    if (!verifyRecordedPayload(*driver))
        return 3;

    return 0;
}
