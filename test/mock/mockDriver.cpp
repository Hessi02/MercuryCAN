#include "mockDriver.hpp"

#include <MercuryCAN/controller/receiver.hpp>

void Can::Driver::MockDriver::transmit(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& payloadLength
) {
    if (!data) 
        return;

    FrameEvent event;
    event.identifier = identifier;
    event.payload.assign(data, data + payloadLength);
    event.timestamp = std::chrono::high_resolution_clock::now();
    txTrace.push_back(event);
}

void Can::Driver::MockDriver::receive(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& payloadLength
) {
    if (_receiver)
        _receiver->processRxData(identifier, data, payloadLength);

    FrameEvent event;
    event.identifier = identifier;
    event.payload.assign(data, data + payloadLength);
    event.timestamp = std::chrono::high_resolution_clock::now();
    rxTrace.push_back(event);
}

void Can::Driver::MockDriver::addRxMessage(
    const uint16_t& identifier, const uint8_t& length
) {

}

__attribute__((noreturn))
void Can::Driver::MockDriver::enterIdleSleep(void) const {
    while (true); 
}

void Can::Driver::MockDriver::removeRxMessage(
    const uint16_t& identifier
) {

}

void Can::Driver::MockDriver::setReceiverInstance(
    Can::Controller::Receiver* recv
) {
    _receiver = recv;
}

uint32_t Can::Driver::MockDriver::getTickCountMs(void) const {
    return _tickCountMs;
}

void Can::Driver::MockDriver::incrementTickCountMs(void) {
    _tickCountMs++;
}

void Can::Driver::MockDriver::injectRxFrame(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& payloadLength
) {
    receive(identifier, data, payloadLength);
}