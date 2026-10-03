#include "driver/driver.hpp"

#include "driver/avrDriver.hpp"

Can::Controller::Driver* Can::Controller::Driver::getInstance(void) {
    static AvrDriver driver;
    return &driver;
}