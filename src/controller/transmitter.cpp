/**
 * \file    transmitter.cpp
 * \brief   Implements immediate and cyclic CAN frame transmission.
 *
 * The Transmitter sends individual frames through the active driver and emits
 * their sent notifications after transmission. It also registers non-owning
 * references to cyclic frames, orders them by CAN identifier, and processes
 * them during timer-driven cycles. Each cycle advances the driver's millisecond
 * tick count, emits pre-send notifications for due frames, transmits them,
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

void Can::Controller::Transmitter::sendFrame(Model::Runtime::Frame& frame) const {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();

    driver->transmit(
        frame.getIdentifier(),
        frame.getPayloadData(),
        frame.getPayloadSize()
    );

    emit frame.sent(driver->getTickCountMs());
}

void Can::Controller::Transmitter::addCyclicFrame(
    Model::Runtime::CyclicFrame& frame
) {
    _cyclicMessages.push_back(&frame);

    std::stable_sort(
        _cyclicMessages.begin(),
        _cyclicMessages.end(),
        [](Model::Runtime::CyclicFrame* const& left,
           Model::Runtime::CyclicFrame* const& right) {
            return left->getIdentifier() < right->getIdentifier();
        }
    );

    _dueMessages.resize(_cyclicMessages.size(), false);
}

std::size_t Can::Controller::Transmitter::getFrameCount(void) const {
    return _cyclicMessages.size();
}

void Can::Controller::Transmitter::processTransmitCycle(void) {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->incrementTickCountMs();

    const uint32_t tickCountMs = driver->getTickCountMs();
    const std::size_t messageCount = _cyclicMessages.size();

    for (std::size_t index = 0; index < messageCount; index++)
        processFrame(index, tickCountMs, _dueMessages);

    for (std::size_t index = 0; index < messageCount; index++)
        transmitFrame(index, _dueMessages);

    for (std::size_t index = 0; index < messageCount; index++)
        emitSentFrame(index, tickCountMs, _dueMessages);
}

void Can::Controller::Transmitter::processFrame(
    const std::size_t& index,
    const uint32_t& tickCountMs,
    std::vector<bool>& dueMessages
) {
    Model::Runtime::CyclicFrame& frame = *_cyclicMessages.at(index);
    const uint16_t cycleTime = frame.getCycleTime();
    
    if (0 == cycleTime || 0 != tickCountMs % cycleTime) {
        dueMessages[index] = false;
        return;
    }

    dueMessages[index] = true;
    emit frame.preSend(tickCountMs);
}

void Can::Controller::Transmitter::transmitFrame(
    const std::size_t& index,
    const std::vector<bool>& dueMessages
) {
    if (!dueMessages[index])
        return;

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    Model::Runtime::CyclicFrame& frame = *_cyclicMessages.at(index);
    driver->transmit(
        frame.getIdentifier(),
        frame.getPayloadData(),
        frame.getPayloadSize()
    );
}

void Can::Controller::Transmitter::emitSentFrame(
    const std::size_t& index,
    const uint32_t& tickCountMs,
    const std::vector<bool>& dueMessages
) {
    if (dueMessages[index])
        emit _cyclicMessages.at(index)->sent(tickCountMs);
}