/**
 * \file    relays.cpp (relays example)
 * \brief   Receives cyclic relay control messages and drives PORTA.
 *
 * Configures PORTA as an output and registers the DBC-defined RelayControl
 * message for reception with a 1 ms cycle time. Payload data is received into
 * RAM and copied to PORTA by a callback. The application then enters idle sleep
 * and relies on CAN interrupts to process incoming messages.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <MercuryCAN/model/database.hpp>
#include <MercuryCAN/driver/driverInterface.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>

#include <avr/io.h>
#include <util/delay.h>
#include <cassert>

namespace {

volatile uint8_t relayControlValue = 0;

void updateRelayOutputs(uint32_t) {
    PORTA = relayControlValue;
}

}

int main(void) {
    DDRA = 0xff;
   
    _delay_ms(100);

    constexpr Can::Model::Database dbc(DBC);

    Can::Model::CyclicMessage relayControl = 
        dbc.buildCyclicMessage(
            "RelayControl", 
            &relayControlValue
        );

    SignalSlot::MetaObject::connect(
        &relayControl,
        &Can::Model::Message::received,
        &updateRelayOutputs
    );

    Can::Controller::Receiver rx;
    rx.addCyclicMessage(relayControl);

    auto* driver = Can::Driver::DriverInterface::getInstance();
    driver->enterIdleSleep();
}