#include <MercuryCAN/driver/driver.hpp>
#include <MercuryCAN/driver/avrDriver.hpp>

Can::Controller::Driver* Can::Controller::Driver::getInstance(void) {
    static AvrDriver driver;
    return &driver;
}