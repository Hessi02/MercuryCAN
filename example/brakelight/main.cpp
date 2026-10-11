/**
 * \file    main.cpp (brakelight example)
 * \brief   Demonstrates a CAN-controlled brake light.
 *
 * Receives brake pressure measurements and thresholds, then updates the
 * brake light state when either measured pressure exceeds its threshold.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>

#include <avr/io.h>

int main(void) {
    volatile uint8_t  brakelightState = 0;

    volatile uint16_t brakePressureFrontMPa;
    volatile uint16_t brakePressureRearMPa;
    
    volatile uint16_t brakePressureThresholdFrontMPa;
    volatile uint16_t brakePressureThresholdRearMPa;

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
        &brakePressureThresholdFrontMPa,
        &brakePressureThresholdRearMPa
    );

    rx.awaitMessage(rxBrakeThreshold);
    brakelightState |= 1;
    DDRA |= 1;

    while (true) {
        if (brakePressureThresholdFrontMPa < brakePressureFrontMPa || 
            brakePressureThresholdRearMPa  < brakePressureRearMPa
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