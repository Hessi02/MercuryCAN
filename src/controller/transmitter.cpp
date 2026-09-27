#include "transmitter.hpp"

#include "controller/driver.hpp"

#include <avr/interrupt.h>
#include <avr/io.h>

Can::Controller::Transmitter::Transmitter(void) {
    Driver& driver = Driver::getInstance();
    driver.activateTxTimer();
}

void Can::Controller::Transmitter::sendMessage(Model::Message& message) const {
    Driver& driver = Driver::getInstance();

    driver.transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );

    emit message.sent(driver.getTickCountMs());
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

unsigned char Can::Controller::Transmitter::getMessageCount(void) const {
    return _messageCount;
}

void Can::Controller::Transmitter::processTransmitCycle(void) {
    Driver& driver = Driver::getInstance();
    driver.incrementTickCountMs();

    const unsigned long tickCountMs = driver.getTickCountMs();
    bool dueMessages[_messageCount] = {};

    for (unsigned char index = 0; index < _messageCount; index++)
        processMessage(index, tickCountMs, dueMessages);

    for (unsigned char index = 0; index < _messageCount; index++)
        transmitMessage(index, dueMessages);

    for (unsigned char index = 0; index < _messageCount; index++)
        emitSentMessage(index, tickCountMs, dueMessages);
}

void Can::Controller::Transmitter::processMessage(
    const unsigned char& index,
    const unsigned long& tickCountMs,
    bool* dueMessages
) {
    Model::CyclicMessage& message = *_cyclicMessages.at(index);
    const unsigned short cycleTime = message.getCycleTime();

    if (0 == cycleTime || 0 != tickCountMs % cycleTime)
        return;

    dueMessages[index] = true;
    emit message.preSend(tickCountMs);
}

void Can::Controller::Transmitter::transmitMessage(
    const unsigned char& index, const bool* dueMessages
) {
    if (!dueMessages[index])
        return;

    Driver& driver = Driver::getInstance();
    Model::CyclicMessage& message = *_cyclicMessages.at(index);
    driver.transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );
}

void Can::Controller::Transmitter::emitSentMessage(
    const unsigned char& index,
    const unsigned long& tickCountMs,
    const bool* dueMessages
) {
    if (dueMessages[index])
        emit _cyclicMessages.at(index)->sent(tickCountMs);
}

ISR(TIMER0_COMP_vect) {
    Can::Controller::Transmitter::processTransmitCycle();
}