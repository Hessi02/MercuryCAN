#include <MercuryCAN/driver/driverInterface.hpp>

#ifdef __USE_MOCK_DRIVER
    #include <MercuryCAN/test/mockDriver.hpp>
#else
    #include <MercuryCAN/driver/avrDriver.hpp>
#endif

Can::Driver::DriverInterface* Can::Driver::DriverInterface::getInstance(void) {
#ifdef __USE_MOCK_DRIVER
    static Can::Driver::MockDriver driver;
#else
    static Can::Driver::AvrDriver driver;
#endif
    return &driver;
}