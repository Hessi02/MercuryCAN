#include "transmitter.hpp"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "controller/driver.hpp"

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
    _cyclicMessages.append(message);
    _messageCount++;
}

unsigned char Can::Controller::Transmitter::getMessageCount(void) const {
    return _messageCount;
}

void Can::Controller::Transmitter::processTransmitCycle(void) {
    Driver& driver = Driver::getInstance();
    driver.incrementTickCountMs();

    for (unsigned char i = 0; i < _messageCount; i++) {
        Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (0 == driver.getTickCountMs() % message.getCycleTime()) {
            Driver& driver = Driver::getInstance();

            driver.transmit(
                message.getIdentifier(),
                message.getPayloadData(),
                message.getPayloadSize()
            );

            emit message.sent(driver.getTickCountMs());
        }
    }
}

ISR(TIMER0_COMP_vect) {
    Can::Controller::Transmitter::processTransmitCycle();
}