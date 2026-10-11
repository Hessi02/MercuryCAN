#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>

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

void Can::Driver::MockDriver::activateTxTimer(void) {
    std::lock_guard<std::mutex> lock(_txTimerMutex);

    if (_txTimerThread.joinable())
        return;

    _txTimerRunning.store(true);
    try {
        _txTimerThread = std::thread([this]() {
            using Clock = std::chrono::steady_clock;
            constexpr auto cyclePeriod = std::chrono::milliseconds(1);
            auto nextCycle = Clock::now();

            while (_txTimerRunning.load()) {
                nextCycle += cyclePeriod;
                std::this_thread::sleep_until(nextCycle);

                if (!_txTimerRunning.load())
                    break;

                Can::Controller::Transmitter::processTransmitCycle();

                const auto now = Clock::now();
                if (nextCycle < now)
                    nextCycle = now;
            }
        });
    } catch (...) {
        _txTimerRunning.store(false);
        throw;
    }
}

void Can::Driver::MockDriver::stopTxTimer(void) {
    std::lock_guard<std::mutex> lock(_txTimerMutex);
    _txTimerRunning.store(false);

    if (_txTimerThread.joinable())
        _txTimerThread.join();
}

Can::Driver::MockDriver::~MockDriver() {
    stopTxTimer();
}

void Can::Driver::MockDriver::setReceiverInstance(
    Can::Controller::Receiver* recv
) {
    _receiver = recv;
}

uint32_t Can::Driver::MockDriver::getTickCountMs(void) const {
    return _tickCountMs.load();
}

void Can::Driver::MockDriver::incrementTickCountMs(void) {
    _tickCountMs.fetch_add(1);
}

void Can::Driver::MockDriver::injectRxFrame(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& payloadLength
) {
    receive(identifier, data, payloadLength);
}