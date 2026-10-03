#include "controller/driver.hpp"

#include "controller/avrDriver.hpp"

Can::Controller::Driver* Can::Controller::Driver::getInstance(void) {
    static AvrDriver driver;
    return &driver;
}