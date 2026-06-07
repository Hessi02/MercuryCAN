#include "driver.hpp"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "controller/receiver.hpp"
#include "controller/transmitter.hpp"

Can::Controller::Driver& Can::Controller::Driver::getInstance(void) {
    static Driver instance;
    return instance;
}

void Can::Controller::Driver::transmit(
    const unsigned short& identifier,
    const unsigned char* data,
    const std::size_t& payloadLength
) {
    if (data) {
        unsigned char messageObject = reserveMessageObject();

        if (0 > messageObject)
            return;

        CANPAGE = (messageObject << 4) & 0xff;

        CANIDT1 = (unsigned char)(identifier >> 3);
        CANIDT2 = (unsigned char)((identifier & 0x07) << 5);

        CANIDM1 = 0x00;
        CANIDM2 = 0x00;
        CANIDM3 = 0x00;
        CANIDM4 = 0x00;

        for (std::size_t i = 0; i < payloadLength; i++)
            CANMSG = data[i];

        CANCDMOB |= (1 << CONMOB0) | (payloadLength & 0x0f);

        while (!(CANSTMOB & (1 << TXOK)))
            ;

        CANSTMOB = 0x00;
        CANCDMOB = 0x00;

        freeMessageObject(messageObject);
    }
}

void Can::Controller::Driver::receive(
    const unsigned short& identifier,
    const unsigned char* data,
    const std::size_t& payloadLength
) {
    if (_receiver)
        _receiver->processRxData(identifier, data, payloadLength);
}

void Can::Controller::Driver::addRxMessage(
    const unsigned short& identifier, const unsigned char& length
) {
    unsigned char messageObject = reserveMessageObject();

    CANPAGE = (messageObject << 4);

    CANIDT1 = (unsigned char)(identifier >> 3);
    CANIDT2 = (unsigned char)(identifier << 5);

    CANIDM1 = 0xFF;
    CANIDM2 = 0xE0;

    CANCDMOB = (length & 0x0F);
    CANCDMOB |= (1 << CONMOB1);

    if (messageObject < 8)
        CANIE2 |= (1 << messageObject);
    else
        CANIE1 |= (1 << (messageObject - 8));
}

void Can::Controller::Driver::removeRxMessage(
    const unsigned short& identifier
) {
    for (unsigned char index = 0; index < _messageObjectCount; index++) {
        CANPAGE = (index << 4);

        unsigned short id =
            ((unsigned short)CANIDT1 << 3) | ((unsigned short)CANIDT2 >> 5);

        if (identifier == id) {
            freeMessageObject(index);

            if (index < 8)
                CANIE2 &= ~(1 << index);
            else
                CANIE1 &= ~(1 << (index - 8));
        }
    }
}

void Can::Controller::Driver::setReceiverInstance(Receiver* recv) {
    _receiver = recv;
}

void Can::Controller::Driver::initHardware(void) const {
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

void Can::Controller::Driver::activateTxTimer(void) {
    TCCR0A = (1 << WGM01) | (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
    sei();
}

unsigned char Can::Controller::Driver::reserveMessageObject(void) {
    for (unsigned int i = 0; i < _messageObjectCount; i++) {
        if (0 == (_usedMessageObjectMask & (1 << i))) {
            _usedMessageObjectMask |= (1 << i);
            return i;
        }
    }

    return -1;
}

void Can::Controller::Driver::freeMessageObject(const unsigned char& index) {
    if (0 > index)
        return;

    _usedMessageObjectMask &= ~(1 << index);
}

void Can::Controller::Driver::resetMessageObjects(void) const {
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

Can::Controller::Driver::Driver(void) {
    initHardware();
    activateTxTimer();
}

ISR(TIMER0_COMP_vect) {
    Can::Controller::Transmitter::processTransmitCycle();
}

ISR(CANIT_vect) {
    unsigned char messageObject = CANHPMOB >> 4;

    CANPAGE = messageObject << 4;

    if (CANSTMOB & (1 << RXOK)) {
        unsigned short identifier =
            ((unsigned short)CANIDT1 << 3) | ((unsigned short)CANIDT2 >> 5);

        unsigned char length = CANCDMOB & 0x0F;

        unsigned char data[8];

        for (unsigned char i = 0; i < length; ++i)
            data[i] = CANMSG;

        CANSTMOB = 0x00;
        CANCDMOB = (1 << CONMOB1) | (length & 0x0F);

        Can::Controller::Driver& driver =
            Can::Controller::Driver::getInstance();
        driver.receive(identifier, data, length);
    } else {
        CANSTMOB = 0x00;
        CANCDMOB = (1 << CONMOB1);
    }

    CANGIT |= (1 << CANIT);
}