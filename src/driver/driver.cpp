#include <MercuryCAN/driver/driverInterface.hpp>
#include <MercuryCAN/driver/avrDriver.hpp>

Can::Controller::DriverInterface* Can::Controller::DriverInterface::getInstance(void) {
    static AvrDriver driver;
    return &driver;
}