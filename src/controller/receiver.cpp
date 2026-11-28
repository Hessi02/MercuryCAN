/**
 * \file    receiver.cpp
 * \brief   Implements the CAN frame receiver controller.
 *
 * The Receiver waits for individual frames, optionally with a timeout, and
 * registers cyclic frames for ongoing reception. It configures the active
 * driver for the requested identifiers and processes incoming frames by
 * updating matching frames, setting their update flags, and emitting their
 * received notifications. The receiver stores non-owning pointers to awaited
 * and cyclic frames; those frames must remain valid while in use.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/controller/receiver.hpp>

#include <MercuryCAN/driver/driverInterface.hpp>

void Can::Controller::Receiver::awaitFrame(Model::Runtime::Frame& frame) {
    frame.setUpdateFlag(false);

    _awaitMessageReceived = false;
    _awaitedMessage = &frame;
    
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxFrame(frame.getIdentifier(), frame.getPayloadSize());

    while (!_awaitMessageReceived);

    _awaitedMessage = nullptr;
    driver->removeRxFrame(frame.getIdentifier());
}

bool Can::Controller::Receiver::awaitFrame(
    Model::Runtime::Frame& frame,
    const uint32_t& timeoutMs
) {
    frame.setUpdateFlag(false);

    _awaitMessageReceived = false;
    _awaitedMessage = &frame;

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxFrame(frame.getIdentifier(), frame.getPayloadSize());

    if (timeoutMs > 0)
        driver->activateTxTimer();

    const uint32_t startTick = driver->getTickCountMs();
    while (!_awaitMessageReceived &&
           driver->getTickCountMs() - startTick < timeoutMs) {
    }

    const bool received = _awaitMessageReceived;
    _awaitedMessage = nullptr;
    driver->removeRxFrame(frame.getIdentifier());

    return received;
}

void Can::Controller::Receiver::addCyclicFrame(
    Model::Runtime::CyclicFrame& frame
) {
    _cyclicMessages.push_back(&frame);

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->addRxFrame(frame.getIdentifier(), frame.getPayloadSize());
    driver->setReceiverInstance(this);
}

void Can::Controller::Receiver::processRxData(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& dataLength
) {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    Model::Runtime::Frame* awaitedFrame = _awaitedMessage;
    bool awaitedFrameProcessed = false;

    for (std::size_t i = 0; i < _cyclicMessages.size(); i++) {
        Can::Model::Runtime::CyclicFrame& frame = *_cyclicMessages.at(i);

        if (identifier == frame.getIdentifier()) {
            frame.setPayloadData(data, dataLength);
            frame.setUpdateFlag(true);
            
            emit frame.received(driver->getTickCountMs());

            if (&frame == awaitedFrame)
                awaitedFrameProcessed = true;
        }
    }

    if (awaitedFrame && identifier == awaitedFrame->getIdentifier()) {
        if (!awaitedFrameProcessed) {
            awaitedFrame->setPayloadData(data, dataLength);
            awaitedFrame->setUpdateFlag(true);

            emit awaitedFrame->received(driver->getTickCountMs());
        }

        _awaitMessageReceived = true;
    }
}