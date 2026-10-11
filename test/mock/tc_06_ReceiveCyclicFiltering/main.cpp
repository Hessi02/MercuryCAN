/**
 * \file    main.cpp (tc_06_ReceiveCyclicFiltering)
 * \brief   Tests cyclic receive identifier filtering and notifications.
 *
 * This testcase checks whether an independent and unrelated message reception 
 * changes the configured cyclic RX message. The test checks that only the 
 * configured frame updates the signal data and sets the update flag.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <iostream>

static unsigned int receivedCount = 0;
static uint32_t receivedTick = 0;

void onReceived(uint32_t tickCountMs) {
    receivedCount++;
    receivedTick = tickCountMs;
}

int main() {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    constexpr uint16_t identifier = 0x123;
    volatile uint8_t value = 0;
    
    Can::Model::CyclicMessage frame(
        identifier, 
        10, 
        &value
    );
        
    SignalSlot::MetaObject::connect(
        &frame,
        &Can::Model::Message::received,
        &onReceived
    );
    
    Can::Controller::Receiver rx;
    rx.addCyclicMessage(frame);
        
    constexpr uint16_t unrelatedIdentifier = 0x124;
    constexpr uint8_t unrelatedData = 0xaa;
    
    driver->injectRxFrame(
        unrelatedIdentifier, 
        &unrelatedData, 
        sizeof(unrelatedData)
    );

    if (0 != value || frame.getUpdateFlag() || 0 != receivedCount) {
        std::cerr << "Frame with an unrelated identifier changed the message!"
                  << std::endl;
                  
        return 1;
    }

    constexpr uint8_t expectedValue = 0x5a;

    driver->injectRxFrame(
        identifier, 
        &expectedValue, 
        sizeof(expectedValue)
    );
    
    if (expectedValue != value) {
        std::cerr << "Matching frame payload was not applied!" << std::endl;

        return 2;
    }

    if (!frame.getUpdateFlag()) {
        std::cerr << "Matching frame did not set the update flag!" << std::endl;

        return 3;
    }

    if (1 != receivedCount || driver->getTickCountMs() != receivedTick) {
        std::cerr << "Matching frame did not emit the expected notification!"
                  << std::endl;

        return 4;
    }

    return 0;
}
