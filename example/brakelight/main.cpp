#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>

#include <avr/io.h>

int main(void) {
    volatile uint8_t  brakelightState = 0;
    volatile uint16_t brakePressureFrontMPa;
    volatile uint16_t brakePressureRearMPa;
    volatile uint16_t brakePressureThreshold;

    Can::Controller::Receiver rx;
    Can::Controller::Transmitter tx;

    Can::Model::CyclicMessage txBrakelightState(
        0x11f,
        100,
        &brakelightState
    );

    tx.addCyclicMessage(txBrakelightState);

    Can::Model::CyclicMessage rxBrakeMeas(
        0x41,
        100,
        &brakePressureFrontMPa,
        &brakePressureRearMPa
    );

    rx.addCyclicMessage(rxBrakeMeas);
    
    Can::Model::Message rxBrakeThreshold(
        0x120,
        &brakePressureThreshold
    );

    rx.awaitMessage(rxBrakeThreshold);
    brakelightState |= 1;
    DDRA |= 1;

    while (true) {
        if (brakePressureThreshold < brakePressureFrontMPa || 
            brakePressureThreshold < brakePressureRearMPa
        ) {
            brakelightState |= (1 << 1);
            PORTA |= 1;
        } else {
            brakelightState &= ~(1 << 1);
            PORTA &= ~1;
        }   
    }

    return 0;
}