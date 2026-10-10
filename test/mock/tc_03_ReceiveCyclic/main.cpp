/**
 * \file    main.cpp (tc_03_ReceiveCyclic)
 * \brief   Tests the Receiver's cyclic receive mechanism using mock driver. 
 *
 * The test firstly initializes a typical CyclicMessage, a Receiver and get's 
 * the singleton MockDriver class. Then the injected RX message input is 
 * compared to the observed data output. If the message is received correctly, 
 * the test returns successful - output to stderr if not.
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

    constexpr uint16_t identifier = 0x123;
    uint8_t byte = 0;

    Can::Model::CyclicMessage rxFrame(
        identifier,
        10,
        &byte
    );

    Can::Controller::Receiver rx;
    rx.addCyclicMessage(rxFrame);

    for (uint8_t i = 0; i < 0xff; i++) {
        driver->injectRxFrame(
            identifier,
            &i,
            sizeof(i)
        );

        if (i + 1 != driver->rxTrace.size()) {
            std::cerr << "Frame(s) in trace may got lost!" << std::endl;
            return 1;
        }

        if (byte != i) {
            std::cerr << "Not received injected byte correctly!" << std::endl;
            return 2;
        }

        if (identifier != driver->rxTrace.back().identifier) {
            std::cerr << "Received invalid identifier correctly!" << std::endl;
            return 3;
        }
    }

    return 0;
}