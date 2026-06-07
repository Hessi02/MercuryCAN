#include "receiver.hpp"

#include "controller/driver.hpp"

void Can::Controller::Receiver::addCyclicMessage(
    Model::CyclicMessage& message
) {
    _cyclicMessages.append(message);
    _messageCount++;

    Driver& driver = Driver::getInstance();
    driver.addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver.addReceiverInstance(this);
}

void Can::Controller::Receiver::processRxData(
    const unsigned short& identifier,
    const unsigned char* data,
    const std::size_t& dataLength
) {
    for (int i = 0; i < _messageCount; i++) {
        Can::Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (identifier == message.getIdentifier())
            message.setPayloadData(data, dataLength);
    }
}