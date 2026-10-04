#include "../mockDriver.hpp"

#include <MercuryCAN/controller/receiver.hpp>

#include <cassert>
#include <limits>

#define __USE_MOCK_DRIVER true

using namespace Can::Model;
using namespace Can::Controller;
using namespace Can::Driver;

int main(void) {
    constexpr uint16_t identifier = 0x123;
    uint64_t receivedData  = 0;

    CyclicMessage msg(
        identifier,
        receivedData
    );

    Receiver rx;
    rx.addCyclicMessage(msg);

    MockDriver* driver = static_cast<MockDriver*>(DriverInterface::getInstance());
    
    for (uint64_t i = 0; i < std::numeric_limits<uint64_t>::max(); i++) {
        driver->injectRxFrame(
            identifier, 
            static_cast<uint8_t*>(&i), 
            sizeof(i)
        );

        assert(receivedData == i);
    }
}