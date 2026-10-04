#include <MercuryCAN/driver/driverInterface.hpp>
#include <MercuryCAN/driver/avrDriver.hpp>

Can::Driver::DriverInterface* Can::Driver::DriverInterface::getInstance(void) {
#ifdef __USE_MOCK_DRIVER
    static Can::Driver::MockDriver driver;
#else
    static Can::Driver::AvrDriver driver;
#endif
    return &driver;
}