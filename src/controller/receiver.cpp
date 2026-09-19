#include "receiver.hpp"

#include <util/delay.h>
#include <controller/driver.hpp>

bool Can::Controller::Receiver::awaitMessage(Model::Message& message, const unsigned int& timeoutMs) {
    Driver& driver = Driver::getInstance();
    driver.addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver.setReceiverInstance(this);

    unsigned int iterator = 0;
    bool messageReceived = true;

    while (!message.getUpdateFlag()) {
        if (timeoutMs > 0 && iterator++ >= timeoutMs) {
            messageReceived = false;
            break;
        } 
        
        _delay_ms(1);
    }

    driver.removeRxMessage(message.getIdentifier());
    return messageReceived;
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
    Driver& driver = Driver::getInstance();
    
    for (int i = 0; i < _messageCount; i++) {
        Can::Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (identifier == message.getIdentifier()) {
            message.setPayloadData(data, dataLength);
            message.setUpdateFlag(true);
            
            emit message.received(driver.getTickCountMs());
        }
    }
}