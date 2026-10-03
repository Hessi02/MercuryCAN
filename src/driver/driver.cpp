#include <MercuryCAN/driver/driverInterface.hpp>
#include <MercuryCAN/driver/avrDriver.hpp>

Can::Driver::DriverInterface* Can::Driver::DriverInterface::getInstance(void) {
    static Can::Driver::AvrDriver driver;
    return &driver;
}