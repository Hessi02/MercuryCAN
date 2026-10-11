#include <MercuryCAN/driver/driverInterface.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>

#include <avr/io.h>

int main(void) {
    Can::Controller::Receiver rx;
    Can::Controller::Transmitter tx;

    DDRA = 0xff;
    DDRB = 0xff;

    Can::Model::CyclicMessage rxControl(
        0x001,
        10,
        &PORTA,
        &PORTB
    );
    
    rx.addCyclicMessage(rxControl);

    Can::Model::CyclicMessage txState(
        0x002,
        10,
        &PORTA,
        &PORTB
    );

    tx.addCyclicMessage(txState);

    auto* driver = Can::Driver::DriverInterface::getInstance();
    driver->enterIdleSleep();
}