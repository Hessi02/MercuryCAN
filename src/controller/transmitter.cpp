/**
 * \file    transmitter.cpp
 * \brief   Implements immediate and cyclic CAN message transmission.
 *
 * The Transmitter sends individual messages through the active driver and emits 
 * their sent notifications after transmission. It also registers non-owning 
 * references to cyclic messages, orders them by CAN identifier, and processes 
 * them during timer-driven cycles. Each cycle advances the driver's millisecond 
 * tick count, emits pre-send notifications for due messages, transmits them, 
 * and then emits their sent notifications.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/controller/transmitter.hpp>

#include <MercuryCAN/driver/driverInterface.hpp>

#include <algorithm>

Can::Controller::Transmitter::Transmitter(void) {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->activateTxTimer();
}

void Can::Controller::Transmitter::sendMessage(Model::Runtime::Message& message) const {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();

    driver->transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );

    emit message.sent(driver->getTickCountMs());
}

void Can::Controller::Transmitter::addCyclicMessage(
    Model::Runtime::CyclicMessage& message
) {
    _cyclicMessages.push_back(&message);

    std::stable_sort(
        _cyclicMessages.begin(),
        _cyclicMessages.end(),
        [](Model::Runtime::CyclicMessage* const& left,
           Model::Runtime::CyclicMessage* const& right) {
            return left->getIdentifier() < right->getIdentifier();
        }
    );

    _dueMessages.resize(_cyclicMessages.size(), false);
}

std::size_t Can::Controller::Transmitter::getMessageCount(void) const {
    return _cyclicMessages.size();
}

void Can::Controller::Transmitter::processTransmitCycle(void) {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->incrementTickCountMs();

    const uint32_t tickCountMs = driver->getTickCountMs();
    const std::size_t messageCount = _cyclicMessages.size();

    for (std::size_t index = 0; index < messageCount; index++)
        processMessage(index, tickCountMs, _dueMessages);

    for (std::size_t index = 0; index < messageCount; index++)
        transmitMessage(index, _dueMessages);

    for (std::size_t index = 0; index < messageCount; index++)
        emitSentMessage(index, tickCountMs, _dueMessages);
}

void Can::Controller::Transmitter::processMessage(
    const std::size_t& index,
    const uint32_t& tickCountMs,
    std::vector<bool>& dueMessages
) {
    Model::Runtime::CyclicMessage& message = *_cyclicMessages.at(index);
    const uint16_t cycleTime = message.getCycleTime();
    
    if (0 == cycleTime || 0 != tickCountMs % cycleTime) {
        dueMessages[index] = false;
        return;
    }

    dueMessages[index] = true;
    emit message.preSend(tickCountMs);
}

void Can::Controller::Transmitter::transmitMessage(
    const std::size_t& index,
    const std::vector<bool>& dueMessages
) {
    if (!dueMessages[index])
        return;

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    Model::Runtime::CyclicMessage& message = *_cyclicMessages.at(index);
    driver->transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );
}

void Can::Controller::Transmitter::emitSentMessage(
    const std::size_t& index,
    const uint32_t& tickCountMs,
    const std::vector<bool>& dueMessages
) {
    if (dueMessages[index])
        emit _cyclicMessages.at(index)->sent(tickCountMs);
}