/**
 * \file    relays.cpp (relays example)
 * \brief   Receives cyclic relay control messages and drives PORTA.
 *
 * Configures PORTA as an output and registers the DBC-defined RelayControl
 * message for reception with a 1 ms cycle time. The received payload is
 * written to PORTA by the receiver. The application then enters idle sleep
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

int main(void) {
    DDRA = 0xff;
   
    _delay_ms(100);

    constexpr Can::Model::Database dbc(DBC);

    std::optional<Can::Model::CyclicMessage> relayControl = 
        dbc.buildCyclicMessage(
            "RelayControl",
            1,
            &PORTA
        );

    Can::Controller::Receiver rx;
    
    if (relayControl)
        rx.addCyclicMessage(*relayControl);

    auto* driver = Can::Driver::DriverInterface::getInstance();
    driver->enterIdleSleep();
}