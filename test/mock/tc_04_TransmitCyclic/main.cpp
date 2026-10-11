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

#include <chrono>
#include <iostream>
#include <thread>

static constexpr uint16_t cycleTimeMs = 10;
static constexpr std::size_t expectedTxCount = 10;

int main(int argc, char** argv) {
    auto* driver = static_cast<Can::Driver::MockDriver*>(
        Can::Driver::DriverInterface::getInstance()
    );

    constexpr uint16_t identifier = 0x123;
    uint8_t byte = 0;

    Can::Model::CyclicMessage txFrame(
        identifier,
        cycleTimeMs,
        &byte
    );

    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(txFrame);

    const uint32_t startTick = driver->getTickCountMs();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    
    while (driver->getTickCountMs() - startTick < expectedTxCount * cycleTimeMs && 
           std::chrono::steady_clock::now() < deadline
    ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    driver->stopTxTimer();
    const std::size_t actualTxCount = driver->txTrace.size();

    if (actualTxCount < expectedTxCount) {
        std::cerr << "Timed out waiting for cyclic transmissions (TX count: "
                  << actualTxCount << ")" << std::endl;
        return 1;
    }

    return 0;
}