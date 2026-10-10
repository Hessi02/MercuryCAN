#include <MercuryCAN/controller/receiver.hpp>

#include <MercuryCAN/driver/driverInterface.hpp>

void Can::Controller::Receiver::awaitMessage(Model::Message& message) {
    message.setUpdateFlag(false);

    _awaitMessageReceived.store(false, std::memory_order_relaxed);
    _awaitedMessage.store(&message, std::memory_order_release);
    
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());

    while (!_awaitMessageReceived.load(std::memory_order_acquire));

    _awaitedMessage.store(nullptr, std::memory_order_release);
    driver->removeRxMessage(message.getIdentifier());
}

void Can::Controller::Receiver::addCyclicMessage(
    Model::CyclicMessage& message
) {
    _cyclicMessages.append(&message);
    _messageCount++;

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());
    driver->setReceiverInstance(this);
}

void Can::Controller::Receiver::processRxData(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& dataLength
) {
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    Model::Message* awaitedMessage =
        _awaitedMessage.load(std::memory_order_acquire);
    bool awaitedMessageProcessed = false;

    for (uint8_t i = 0; i < _messageCount; i++) {
        Can::Model::CyclicMessage& message = *_cyclicMessages.at(i);

        if (identifier == message.getIdentifier()) {
            message.setPayloadData(data, dataLength);
            message.setUpdateFlag(true);
            
            emit message.received(driver->getTickCountMs());

            if (&message == awaitedMessage)
                awaitedMessageProcessed = true;
        }
    }

    if (awaitedMessage && identifier == awaitedMessage->getIdentifier()) {
        if (!awaitedMessageProcessed) {
            awaitedMessage->setPayloadData(data, dataLength);
            awaitedMessage->setUpdateFlag(true);

            emit awaitedMessage->received(driver->getTickCountMs());
        }

        _awaitMessageReceived.store(true, std::memory_order_release);
    }
}