/**
 * \file    sender.cpp (relays example)
 * \brief   Periodically sends the relay control state read from PINA.
 *
 * Configures PORTA as an input and registers the DBC-defined RelayControl
 * message for cyclic transmission with a 1 ms cycle time. The message payload
 * is populated from a RAM copy of PINA immediately before transmission. The
 * application then enters idle sleep while the timer interrupt schedules
 * transmissions.
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

namespace {

volatile uint8_t relayControlValue = 0;

void readRelayInputs(uint32_t) {
    relayControlValue = PINA;
}

}

int main(void) {
    DDRA = 0x00;
    
    constexpr Can::Model::Database dbc(DBC);

    auto relayControl = 
        dbc.buildCyclicMessage(
            "RelayControl", 
            1, 
            &relayControlValue
        );

    SignalSlot::MetaObject::connect(
        &relayControl,
        &Can::Model::Message::preSend,
        &readRelayInputs
    );
    
    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(relayControl);

    auto* driver = Can::Driver::DriverInterface::getInstance();
    driver->enterIdleSleep();
}