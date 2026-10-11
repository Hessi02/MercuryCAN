/**
 * \file    main.cpp (tc_13_ReceiveWithTimeout)
 * \brief   Tests successful and expired Receiver::awaitMessage() timeouts.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/message.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    constexpr uint16_t identifier = 0x234;
    constexpr uint8_t receivedByte = 0x5a;
    volatile uint8_t byteToRecv = 0;

    Can::Model::Message rxFrame(
        identifier, 
        &byteToRecv
    );

    Can::Controller::Receiver rx;

    std::thread injector([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        driver->injectRxFrame(
            identifier, 
            &receivedByte, 
            sizeof(receivedByte)
        );
    });

    if (!rx.awaitMessage(rxFrame, 25)) {
        injector.join();
        std::cerr << "Message was not received before timeout!" << std::endl;
        return 1;
    }

    injector.join();

    if (receivedByte != byteToRecv || !rxFrame.getUpdateFlag()) {
        std::cerr << "Received data or update flag is invalid!" << std::endl;
        return 2;
    }

    constexpr uint32_t timeoutMs = 30;
    const auto start = std::chrono::steady_clock::now();

    if (rx.awaitMessage(rxFrame, timeoutMs)) {
        std::cerr << "awaitMessage() succeeded without receiving!" << std::endl;
        return 3;
    }
    
    const auto elapsed = std::chrono::steady_clock::now() - start;

    if (elapsed < std::chrono::milliseconds(timeoutMs)) {
        std::cerr << "awaitMessage() returned before timeout!" << std::endl;
        return 4;
    }

    if (rxFrame.getUpdateFlag()) {
        std::cerr << "Timed-out wait left update flag set!" << std::endl;
        return 5;
    }

    return 0;
}
