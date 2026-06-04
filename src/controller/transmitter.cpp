#include "transmitter.hpp"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "driver.hpp"

Can::Controller::Transmitter::Transmitter(void) {
    activateTimer();
}

void Can::Controller::Transmitter::addMessage(Model::Message& message) const {
    Driver& driver = Driver::getInstance();

    driver.transmit(
        message.getIdentifier(),
        message.getPayloadData(),
        message.getPayloadSize()
    );
}

void Can::Controller::Transmitter::addCyclicMessage(
    Model::CyclicMessage& message
) {
    _cyclicMessages.append(message);
    _messageCount++;
}

unsigned char Can::Controller::Transmitter::getMessageCount(void) const {
    return _messageCount;
}

void Can::Controller::Transmitter::processTransmitCycle(void) {
    for (unsigned char i = 0; i < _messageCount; i++) {
        Model::CyclicMessage& message = _cyclicMessages.at(i);

        if (0 == tickCountMs % message.getCycleTime()) {
            Driver& driver = Driver::getInstance();

            driver.transmit(
                message.getIdentifier(),
                message.getPayloadData(),
                message.getPayloadSize()
            );
        }
    }
}

void Can::Controller::Transmitter::incrementTickCount(void) {
    tickCountMs++;
}

void Can::Controller::Transmitter::activateTimer(void) {
    TCCR0A = (1 << WGM01) | (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
    sei();
}

ISR(TIMER0_COMP_vect) {
    Can::Controller::Transmitter::incrementTickCount();
    Can::Controller::Transmitter::processTransmitCycle();
}