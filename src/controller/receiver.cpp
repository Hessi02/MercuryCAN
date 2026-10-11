#include <MercuryCAN/controller/receiver.hpp>

#include <MercuryCAN/driver/driverInterface.hpp>

void Can::Controller::Receiver::awaitMessage(Model::Message& message) {
    message.setUpdateFlag(false);

    _awaitMessageReceived = false;
    _awaitedMessage = &message;
    
    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());

    while (!_awaitMessageReceived);

    _awaitedMessage = nullptr;
    driver->removeRxMessage(message.getIdentifier());
}

bool Can::Controller::Receiver::awaitMessage(
    Model::Message& message,
    const uint32_t& timeoutMs
) {
    message.setUpdateFlag(false);

    _awaitMessageReceived = false;
    _awaitedMessage = &message;

    Driver::DriverInterface* driver = Driver::DriverInterface::getInstance();
    driver->setReceiverInstance(this);
    driver->addRxMessage(message.getIdentifier(), message.getPayloadSize());

    if (timeoutMs > 0)
        driver->activateTxTimer();

    const uint32_t startTick = driver->getTickCountMs();
    while (!_awaitMessageReceived &&
           driver->getTickCountMs() - startTick < timeoutMs) {
    }

    const bool received = _awaitMessageReceived;
    _awaitedMessage = nullptr;
    driver->removeRxMessage(message.getIdentifier());

    return received;
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
    Model::Message* awaitedMessage = _awaitedMessage;
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

        _awaitMessageReceived = true;
    }
}