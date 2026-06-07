#include "receiver.hpp"

#include "controller/driver.hpp"

void Can::Controller::Receiver::awaitMessage(Model::Message& message) {
    Driver& driver = Driver::getInstance();
    driver.addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver.setReceiverInstance(this);

    while (!message.getUpdateFlag())
        ;

    driver.removeRxMessage(message.getIdentifier());
}

void Can::Controller::Receiver::addCyclicMessage(
    Model::CyclicMessage& message
) {
    _cyclicMessages.append(message);
    _messageCount++;

    Driver& driver = Driver::getInstance();
    driver.addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver.setReceiverInstance(this);
}

void Can::Controller::Receiver::processRxData(
    const unsigned short& identifier,
    const unsigned char* data,
    const std::size_t& dataLength
) {
    for (int i = 0; i < _messageCount; i++) {
        Can::Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (identifier == message.getIdentifier()) {
            message.setPayloadData(data, dataLength);
            message.setUpdateFlag(true);
        }
    }
}