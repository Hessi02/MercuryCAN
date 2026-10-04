#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <cstdio>

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

    for (uint64_t i = 0; i < 1000; i++) {
        uint8_t payload[sizeof(i)];
        for (std::size_t byte = 0; byte < sizeof(i); byte++)
            payload[sizeof(i) - 1 - byte] =
                static_cast<uint8_t>(i >> (byte * 8));

        MockDriver* driver = static_cast<MockDriver*>(DriverInterface::getInstance());
        
        driver->injectRxFrame(
            recvIdentifier,
            payload,
            sizeof(payload)
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

        if (recvData != i) {
            std::fprintf(
                stderr,
                "Received 0x%llx, expected 0x%llx\n",
                static_cast<unsigned long long>(recvData),
                static_cast<unsigned long long>(i)
            );
            return 1;
        }
    }
}