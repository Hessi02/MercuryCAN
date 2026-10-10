/**
 * \file    main.cpp (tc_04_TransmitCyclic)
 * \brief   Tests the Transmitter's cyclic transmit mechanism using mock driver. 
 *
 * The test firstly initializes a typical CyclicMessage, a Transmitter and get's 
 * the singleton MockDriver class. Then a message is incremented and transmitted
 * with an cycle time of 1 ms. If the values are identified correctly in the TX
 * trace of the mock driver instance, the test shall be evaluated as passed. If
 * not, the error will be redirected to stderr.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>
#include <MercuryCAN/support/signalSlot/metaObject.hpp>

#include <iostream>

static volatile uint8_t byte = 0;

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    constexpr uint16_t identifier = 0x123;

    Can::Model::CyclicMessage txFrame(
        identifier,
        1,
        &byte
    );

    SignalSlot::MetaObject::connect(
        &txFrame,
        &Can::Model::CyclicMessage::Message::sent,
        +[](uint32_t) {
            byte++;
        }
    );

    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(txFrame);

    while (byte < 128);

    if (driver->txTrace.size() != byte + 1) {
        std::cerr << "At least one message was missed!" << std::endl;
        return 1;
    }

    for (Can::Driver::MockDriver::FrameEvent frame : driver->txTrace) {
        if (identifier != frame.identifier) {
            std::cerr << "Wrong identifier was sent on the bus!" << std::endl;
            return 2;
        }
    }

    // Todo: Check the cycle time. 

    return 0;
}