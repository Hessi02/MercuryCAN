#include <MercuryCAN/controller/receiver.hpp>

#include <MercuryCAN/driver/driverInterface.hpp>

void Can::Controller::Receiver::awaitMessage(Model::Message& message) {
    message.setUpdateFlag(false);
    
    DriverInterface* driver = DriverInterface::getInstance();
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver->setReceiverInstance(this);

    while (!message.getUpdateFlag()); 

    driver->removeRxMessage(message.getIdentifier());
}

void Can::Controller::Receiver::addCyclicMessage(
    Model::CyclicMessage& message
) {
    _cyclicMessages.append(message);
    _messageCount++;

    DriverInterface* driver = DriverInterface::getInstance();
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver->setReceiverInstance(this);
}

void Can::Controller::Receiver::processRxData(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& dataLength
) {
    DriverInterface* driver = DriverInterface::getInstance();

    for (uint8_t i = 0; i < _messageCount; i++) {
        Can::Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (identifier == message.getIdentifier()) {
            message.setPayloadData(data, dataLength);
            message.setUpdateFlag(true);
            
            emit message.received(driver->getTickCountMs());
        }
    }
}