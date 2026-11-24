/**
 * \file    receiver.cpp
 * \brief   Implements the CAN message receiver controller.
 *
 * The Receiver waits for individual messages, optionally with a timeout, and
 * registers cyclic messages for ongoing reception. It configures the active
 * driver for the requested identifiers and processes incoming frames by
 * updating matching messages, setting their update flags, and emitting their
 * received notifications. The receiver stores non-owning pointers to awaited
 * and cyclic messages; those messages must remain valid while in use.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/controller/receiver.hpp>

#include <MercuryCAN/driver/driverInterface.hpp>

void Can::Controller::Receiver::awaitMessage(Model::Runtime::Message& message) {
    message.setUpdateFlag(false);

    _awaitMessageReceived = false;
    _awaitedMessage = &message;
    
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());

    while (!_awaitMessageReceived);

    _awaitedMessage = nullptr;
    driver->removeRxMessage(message.getIdentifier());
}

bool Can::Controller::Receiver::awaitMessage(
    Model::Runtime::Message& message,
    const uint32_t& timeoutMs
) {
    message.setUpdateFlag(false);

    _awaitMessageReceived = false;
    _awaitedMessage = &message;

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());

    if (timeoutMs > 0)
        driver->activateTxTimer();

    const uint32_t startTick = driver->getTickCountMs();
    while (!_awaitMessageReceived &&
           driver->getTickCountMs() - startTick < timeoutMs) {
    }

    const bool received = _awaitMessageReceived;
    _awaitedMessage = nullptr;
    driver->removeRxMessage(message.getIdentifier());

    return received;
}

void Can::Controller::Receiver::addCyclicMessage(
    Model::Runtime::CyclicMessage& message
) {
    _cyclicMessages.push_back(&message);

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver->setReceiverInstance(this);
}

void Can::Controller::Receiver::processRxData(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& dataLength
) {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    Model::Runtime::Message* awaitedMessage = _awaitedMessage;
    bool awaitedMessageProcessed = false;

    for (std::size_t i = 0; i < _cyclicMessages.size(); i++) {
        Can::Model::Runtime::CyclicMessage& message = *_cyclicMessages.at(i);

        if (identifier == message.getIdentifier()) {
            message.setPayloadData(data, dataLength);
            message.setUpdateFlag(true);
            
            emit message.received(driver->getTickCountMs());

            if (&message == awaitedMessage)
                awaitedMessageProcessed = true;
        }
    }

    if (awaitedMessage && identifier == awaitedMessage->getIdentifier()) {
        if (!awaitedMessageProcessed) {
            awaitedMessage->setPayloadData(data, dataLength);
            awaitedMessage->setUpdateFlag(true);

            emit awaitedMessage->received(driver->getTickCountMs());
        }

        _awaitMessageReceived = true;
    }
}