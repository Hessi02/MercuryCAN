#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <cassert>

using namespace Can::Model;
using namespace Can::Driver;
using namespace Can::Controller;

int main(int argc, char** argv) {
    srand(time(nullptr));

    constexpr uint16_t recvIdentifier = 0x123;
    uint64_t recvData = 0;

    CyclicMessage rxFrame(
        recvIdentifier,
        10,
        &recvData
    );

    Receiver rx;
    rx.addCyclicMessage(rxFrame);

    for (uint64_t i = 0; i < UINT64_MAX; i++) {
        MockDriver* driver = static_cast<MockDriver*>(DriverInterface::getInstance());
        
        driver->injectRxFrame(
            recvIdentifier,
            reinterpret_cast<uint8_t*>(&i),
            sizeof(i)
        );

        uint16_t noiseIdentifier = rand() % 0x7ff;

        if (recvIdentifier == noiseIdentifier)
            noiseIdentifier = 0x100;

        uint64_t noiseData = rand();

        driver->injectRxFrame(
            noiseIdentifier,
            reinterpret_cast<uint8_t*>(&noiseData),
            sizeof(noiseData)
        );

        assert(recvData == i);
    }
}