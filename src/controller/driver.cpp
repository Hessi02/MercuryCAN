#include "driver.hpp"

#include "avrDriver.hpp"

Can::Controller::Driver* Can::Controller::Driver::getInstance(void) {
    static AvrDriver driver;
    return &driver;
}