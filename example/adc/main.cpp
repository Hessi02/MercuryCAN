#include <MercuryCAN.hpp>

#include <avr/io.h>

extern const char dbc[];
static volatile uint16_t voltage = 0;

int main(void) {
    Can::Model::Database db(dbc);  
    Can::Model::CyclicMessage* msg = db.buildCyclicMessage("Measurement", 1, &voltage);

    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    SignalSlot::MetaObject::connect(
        msg,
        &Can::Model::Message::preSend,
        +[](uint32_t){
            ADCSRA |= (1 << ADSC);

            while (ADCSRA & (1 << ADSC));

            voltage = static_cast<uint16_t>(
                (static_cast<uint32_t>(ADC) * 5000U + 511U) / 1023U
            );
        }
    );

    Can::Controller::Transmitter tx;
    tx.addCyclicMessage(*msg);

    Can::Driver::DriverInterface* driver = Can::Driver::DriverInterface::getInstance();
    driver->enterIdleSleep();
}