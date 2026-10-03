#include <MercuryCAN/driver/avrDriver.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

void Can::Driver::AvrDriver::transmit(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& payloadLength
) {
    if (!data) 
        return;

    const uint8_t messageObject = reserveMessageObject();

    if (_messageObjectCount == messageObject)
        return;

    configureTransmitMessage(messageObject, identifier);
    transmitPayload(data, payloadLength);
    completeTransmission(messageObject);
}

void Can::Driver::AvrDriver::receive(
    const uint16_t& identifier,
    const uint8_t* data,
    const std::size_t& payloadLength
) {
    if (_receiver)
        _receiver->processRxData(identifier, data, payloadLength);
}

void Can::Driver::AvrDriver::addRxMessage(
    const uint16_t& identifier, const uint8_t& length
) {
    const uint8_t messageObject = reserveMessageObject();

    if (_messageObjectCount == messageObject)
        return;

    CANPAGE = (messageObject << 4);

    CANIDT1 = static_cast<uint8_t>(identifier >> 3);
    CANIDT2 = static_cast<uint8_t>(identifier << 5);

    CANIDM1 = 0xFF;
    CANIDM2 = 0xE0;

    CANCDMOB = (length & 0x0F);
    CANCDMOB |= (1 << CONMOB1);

    if (messageObject < 8)
        CANIE2 |= (1 << messageObject);
    else
        CANIE1 |= (1 << (messageObject - 8));
}

__attribute__((noreturn))
void Can::Driver::AvrDriver::enterIdleSleep(void) const {
    set_sleep_mode(SLEEP_MODE_IDLE);

    while (true) {
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }
}

void Can::Driver::AvrDriver::removeRxMessage(
    const uint16_t& identifier
) {
    for (uint8_t index = 0; index < _messageObjectCount; index++) {
        CANPAGE = (index << 4);

        const uint16_t id =
            (static_cast<uint16_t>(CANIDT1) << 3) |
            (static_cast<uint16_t>(CANIDT2) >> 5);

        if (identifier == id) {
            freeMessageObject(index);

            if (index < 8)
                CANIE2 &= ~(1 << index);
            else
                CANIE1 &= ~(1 << (index - 8));
        }
    }
}

void Can::Driver::AvrDriver::setReceiverInstance(
    Can::Controller::Receiver* recv
) {
    _receiver = recv;
}

void Can::Driver::AvrDriver::configureTransmitMessage(
    const uint8_t& messageObject,
    const uint16_t& identifier
) const {
    CANPAGE = (messageObject << 4) & 0xff;
    CANIDT1 = static_cast<uint8_t>(identifier >> 3);
    CANIDT2 = static_cast<uint8_t>((identifier & 0x07) << 5);
    CANIDM1 = 0x00;
    CANIDM2 = 0x00;
    CANIDM3 = 0x00;
    CANIDM4 = 0x00;
}

void Can::Driver::AvrDriver::transmitPayload(
    const uint8_t* data, const std::size_t& payloadLength
) const {
    for (std::size_t index = 0; index < payloadLength; index++)
        CANMSG = data[index];

    CANCDMOB = (1 << CONMOB0) | (payloadLength & 0x0f);
}

void Can::Driver::AvrDriver::completeTransmission(
    const uint8_t& messageObject
) {
    while (!(CANSTMOB & (1 << TXOK)))
        ;

    CANSTMOB = 0x00;
    CANCDMOB = 0x00;
    freeMessageObject(messageObject);
}

uint32_t Can::Driver::AvrDriver::getTickCountMs(void) const {
    return _tickCountMs;
}

void Can::Driver::AvrDriver::incrementTickCountMs(void) {
    _tickCountMs++;
}

void Can::Driver::AvrDriver::initHardware(void) const {
    CANGCON = (1 << SWRES);
    CANGCON = (1 << ENASTB);

    while (!(CANGSTA & (1 << ENFG)))
        ;

    CANBT1 = 0x00;
    CANBT2 = (1 << PRS1) | (1 << PRS2);
    CANBT3 =
        (1 << SMP) | (1 << PHS10) | (1 << PHS11) | (1 << PHS20) | (1 << PHS21);

    resetMessageObjects();

    CANGCON &= ~(1 << LISTEN);

    CANGIE = (1 << ENIT) | (1 << ENRX);
    sei();
}

void Can::Driver::AvrDriver::activateTxTimer(void) {
    TCCR0A = (1 << WGM01) | (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
    sei();
}

uint8_t Can::Driver::AvrDriver::reserveMessageObject(void) {
    for (uint8_t i = 0; i < _messageObjectCount; i++) {
        if (0 == (_usedMessageObjectMask & (1 << i))) {
            _usedMessageObjectMask |= (1 << i);
            return i;
        }
    }

    return _messageObjectCount;
}

void Can::Driver::AvrDriver::freeMessageObject(const uint8_t& index) {
    if (_messageObjectCount <= index)
        return;

    _usedMessageObjectMask &= ~(1 << index);
}

void Can::Driver::AvrDriver::resetMessageObjects(void) const {
    for (uint8_t index = 0; index < _messageObjectCount; index++) {
        CANPAGE = (index << 4);
        CANCDMOB = 0;
        CANSTMOB = 0;

        CANIDT1 = 0;
        CANIDT2 = 0;
        CANIDT3 = 0;
        CANIDT4 = 0;

        CANIDM1 = 0;
        CANIDM2 = 0;
        CANIDM3 = 0;
        CANIDM4 = 0;
    }
}

Can::Driver::AvrDriver::AvrDriver(void) {
    initHardware();
}

ISR(CANIT_vect) {
    uint8_t messageObject = CANHPMOB >> 4;

    CANPAGE = messageObject << 4;

    if (CANSTMOB & (1 << RXOK)) {
        const uint16_t identifier =
            (static_cast<uint16_t>(CANIDT1) << 3) |
            (static_cast<uint16_t>(CANIDT2) >> 5);

        uint8_t length = CANCDMOB & 0x0F;

        uint8_t data[8];

        for (uint8_t i = 0; i < length; ++i)
            data[i] = CANMSG;

        CANSTMOB = 0x00;
        CANCDMOB = (1 << CONMOB1) | (length & 0x0F);

        Can::Driver::DriverInterface* driver =
            Can::Driver::DriverInterface::getInstance();
        driver->receive(identifier, data, length);
    } else {
        CANSTMOB = 0x00;
        CANCDMOB = (1 << CONMOB1);
    }

    CANGIT |= (1 << CANIT);
}

ISR(TIMER0_COMP_vect) {
    Can::Controller::Transmitter::processTransmitCycle();
}