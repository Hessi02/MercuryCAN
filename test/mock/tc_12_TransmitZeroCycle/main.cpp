/**
 * \file    main.cpp (tc_12_TransmitZeroCycle)
 * \brief   Tests that a cyclic message with a zero interval is not sent.
 *
 * The transmitter is advanced through several cycles and must neither send
 * the message nor emit its pre-send or sent notifications.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <iostream>

static unsigned int preSendCount = 0;
static unsigned int sentCount = 0;

void connectNotifications(Can::Model::CyclicMessage& frame) {
    SignalSlot::MetaObject::connect(
        &frame,
        &Can::Model::Message::preSend,
        +[](uint32_t) {
            preSendCount++;
        }
    );

    SignalSlot::MetaObject::connect(
        &frame,
        &Can::Model::Message::sent,
        +[](uint32_t) {
            sentCount++;
        }
    );
}

bool verifyNoTransmission(
    const Can::Driver::MockDriver& driver,
    const std::size_t initialTxCount
) {
    if (initialTxCount != driver.txTrace.size()) {
        std::cerr << "Zero-interval message was transmitted!" << std::endl;
        return false;
    }

    if (0 != preSendCount || 0 != sentCount) {
        std::cerr << "Zero-interval message emitted signals!" << std::endl;
        return false;
    }

    return true;
}

void processThreeCycles() {
    for (unsigned int cycle = 0; cycle < 3; cycle++)
        Can::Controller::Transmitter::processTransmitCycle();
}

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    Can::Controller::Transmitter transmitter;
    driver->stopTxTimer();

    constexpr uint16_t identifier = 0x402;
    volatile uint8_t byteToTransmit = 0x5a;

    Can::Model::CyclicMessage frame(
        identifier, 
        0, 
        &byteToTransmit
    );

    connectNotifications(frame);

    transmitter.addCyclicMessage(frame);

    const std::size_t initialTxCount = driver->txTrace.size();

    processThreeCycles();

    if (!verifyNoTransmission(*driver, initialTxCount))
        return 1;

    return 0;
}
