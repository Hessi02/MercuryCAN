/**
 * \file    main.cpp (tc_01_ReceiveSingleShot)
 * \brief   Tests the Receiver::awaitMessage() method using mock driver class. 
 *
 * The test firstly initializes a typical Message, a Receiver and get's the 
 * singleton MockDriver class. Then the awaitMessage() method is called. Another
 * thread is injected beforehand, where 100 ms after the await method call a RX 
 * message gets injected. If the message is received correctly, the test returns 
 * successful - output to stderr if not.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    Can::Controller::Receiver rx;

    constexpr uint16_t identifier = 0x100;
    constexpr uint8_t testData = 0xaa;
    uint8_t byte = 0;

    Can::Model::Message rxFrame(
        identifier,
        &byte
    );

    std::thread injector([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        driver->injectRxFrame(
            identifier,
            &testData,
            sizeof(testData)
        );
    });

    rx.awaitMessage(rxFrame);
    injector.join();

    if (testData != byte) {
        std::cerr << "No valid byte was received!" << std::endl;
        return 1;
    }

    if (!rxFrame.getUpdateFlag()) {
        std::cerr << "Message update flag was not set!" << std::endl;
        return 2;
    }

    if (identifier != driver->rxTrace.back().identifier) {
        std::cerr << "Received frame with wrong identifier!" << std::endl;
        return 3;
    }

    return 0;
}