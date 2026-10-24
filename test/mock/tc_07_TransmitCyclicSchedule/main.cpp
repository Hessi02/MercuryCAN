/**
 * \file    main.cpp (tc_07_TransmitCyclicSchedule)
 * \brief   Tests cyclic transmit intervals and message notifications.
 *
 * This testcase simulates the TX scheduler. It checks whether the transmitter
 * correctly inteprets the driver's system time and evaluates the message's 
 * cycle time correctly.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <array>
#include <iostream>

static std::array<uint32_t, 2> preSendTicks{};
static std::array<uint32_t, 2> sentTicks{};

static std::size_t preSendCount = 0;
static std::size_t sentCount = 0;

int main(int argc, char** argv) {
    Can::Driver::MockDriver* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    Can::Controller::Transmitter tx;
    driver->stopTxTimer();

    constexpr uint16_t identifier = 0x234;
    constexpr uint16_t cycleTimeMs = 2;
    volatile uint8_t byteToTransmit = 0x7b;

    Can::Model::CyclicMessage txFrame(
        identifier, 
        cycleTimeMs, 
        &byteToTransmit
    );

    SignalSlot::MetaObject::connect(
        &txFrame,
        &Can::Model::Message::preSend,
        +[](uint32_t tickCountMs) {
            if (preSendCount < preSendTicks.size())
                preSendTicks[preSendCount] = tickCountMs;

            preSendCount++;
        }
    );

    SignalSlot::MetaObject::connect(
        &txFrame,
        &Can::Model::Message::sent,
        +[](uint32_t tickCountMs) {
            if (sentCount < sentTicks.size())
                sentTicks[sentCount] = tickCountMs;

            sentCount++;
        }
    );

    tx.addCyclicMessage(txFrame);
    const std::size_t initialTxCount = driver->txTrace.size();

    for (unsigned int cycle = 0; cycle < 4; cycle++)
        Can::Controller::Transmitter::processTransmitCycle();

    if (driver->txTrace.size() != initialTxCount + 2) {
        std::cerr << "Count of transmitted frames is not valid!" << std::endl;
        return 1;
    }

    if (2 != preSendCount || 2 != sentCount) {
        std::cerr << "Cyclic message's signal count is not valid!" << std::endl;
        return 2;
    }

    for (std::size_t index = 0; index < sentTicks.size(); index++) {
        if (preSendTicks[index] != sentTicks[index]) {
            std::cerr << "Both signals used different ticks!" << std::endl;
            return 3;
        }

        if (0 != sentTicks[index] % cycleTimeMs) {
            std::cerr << "Sent signal was sent out of interval!" << std::endl;
            return 4;
        }
    }

    for (std::size_t index = initialTxCount; index < driver->txTrace.size(); index++) {
        const Can::Driver::MockDriver::FrameEvent& event = driver->txTrace[index];

        if (identifier != event.identifier          ||
            1 != event.payload.size()               ||
            event.payload.front() != byteToTransmit
        ) {    
            std::cerr << "Transmission contains incorrect data!" << std::endl;
            return 5;
        }
    }

    return 0;
}
