#include "transmitter.hpp"

#include "controller/driver.hpp"

void Can::Controller::Transmitter::sendMessage(Model::Message& message) const {
    Driver& driver = Driver::getInstance();

    driver.transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );
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
    for (unsigned char i = 0; i < _messageCount; i++) {
        Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (0 == _tickCountMs++ % message.getCycleTime()) {
            Driver& driver = Driver::getInstance();

            driver.transmit(
                message.getIdentifier(),
                message.getPayloadData(),
                message.getPayloadSize()
            );
        }
    }
}