/**
 * \file    main.cpp (tc_09_TransmitSentNotification)
 * \brief   Tests the single-shot sent notification and its tick value.
 *
 * This testcase checks the sent signals of an single-shot message. Therefore
 * the count of emitted signals and the passed driver system time are checked. 
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/model/message.hpp>

#include <iostream>

Can::Driver::MockDriver* driver = nullptr;

static unsigned int sentCount        = 0;
static uint32_t sentTick             = 0;
static std::size_t traceSizeWhenSent = 0;

void onSent(uint32_t tickCountMs) {
    sentCount++;
    sentTick = tickCountMs;
    traceSizeWhenSent = driver->txTrace.size();
}

int main() {
    driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    Can::Controller::Transmitter tx;

    constexpr uint16_t identifier = 0x456;
    volatile uint8_t value = 0xa5;
    
    Can::Model::Message frame(
        identifier, 
        &value
    );
    
    SignalSlot::MetaObject::connect(
        &frame,
        &Can::Model::Message::sent,
        &onSent
    );

    const uint32_t expectedTick = driver->getTickCountMs();
    tx.sendMessage(frame);

    if (1 != sentCount) {
        std::cerr << "Single-shot transmission did not emit exactly one "
                  << "sent notification!" << std::endl;

        return 1;
    }

    if (expectedTick != sentTick) {
        std::cerr << "Sent notification carried the wrong tick count!"
                  << std::endl;

        return 2;
    }

    if (1 != traceSizeWhenSent || 1 != driver->txTrace.size()) {
        std::cerr << "Sent notification did not occur after the frame was "
                  << "recorded!" << std::endl;

        return 3;
    }

    const Can::Driver::MockDriver::FrameEvent& event = driver->txTrace.front();

    if (identifier != event.identifier ||
        1 != event.payload.size()      ||
        event.payload.front() != value
    ) {
        std::cerr << "The notified transmission contained incorrect frame "
                  << "data!" << std::endl;

        return 4;
    }

    return 0;
}
