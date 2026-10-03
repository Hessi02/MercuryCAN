#include "controller/transmitter.hpp"

#include "controller/driver.hpp"

Can::Controller::Transmitter::Transmitter(void) {
    Driver* driver = Driver::getInstance();
    driver->activateTxTimer();
}

void Can::Controller::Transmitter::sendMessage(Model::Message& message) const {
    Driver* driver = Driver::getInstance();

    driver->transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );

    emit message.sent(driver->getTickCountMs());
}

void Can::Controller::Transmitter::addCyclicMessage(
    Model::CyclicMessage& message
) {
    _cyclicMessages.append(&message);

    _cyclicMessages.sort(
        [](Model::CyclicMessage* const& left,
           Model::CyclicMessage* const& right) {
            return left->getIdentifier() < right->getIdentifier();
        }
    );

    _messageCount++;
}

uint8_t Can::Controller::Transmitter::getMessageCount(void) const {
    return _messageCount;
}

void Can::Controller::Transmitter::processTransmitCycle(void) {
    Driver* driver = Driver::getInstance();
    driver->incrementTickCountMs();

    const uint32_t tickCountMs = driver->getTickCountMs();
    bool dueMessages[_messageCount] = {};

    for (uint8_t index = 0; index < _messageCount; index++)
        processMessage(index, tickCountMs, dueMessages);

    for (uint8_t index = 0; index < _messageCount; index++)
        transmitMessage(index, dueMessages);

    for (uint8_t index = 0; index < _messageCount; index++)
        emitSentMessage(index, tickCountMs, dueMessages);
}

void Can::Controller::Transmitter::processMessage(
    const uint8_t& index,
    const uint32_t& tickCountMs,
    bool* dueMessages
) {
    Model::CyclicMessage& message = *_cyclicMessages.at(index);
    const uint16_t cycleTime = message.getCycleTime();

    if (0 == cycleTime || 0 != tickCountMs % cycleTime)
        return;

    dueMessages[index] = true;
    emit message.preSend(tickCountMs);
}

void Can::Controller::Transmitter::transmitMessage(
    const uint8_t& index, const bool* dueMessages
) {
    if (!dueMessages[index])
        return;

    Driver* driver = Driver::getInstance();
    Model::CyclicMessage& message = *_cyclicMessages.at(index);
    driver->transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );
}

void Can::Controller::Transmitter::emitSentMessage(
    const uint8_t& index,
    const uint32_t& tickCountMs,
    const bool* dueMessages
) {
    if (dueMessages[index])
        emit _cyclicMessages.at(index)->sent(tickCountMs);
}