#include <MercuryCAN/test/mockDriver.hpp>
#include <MercuryCAN/controller/transmitter.hpp>
#include <MercuryCAN/controller/receiver.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

#include <iostream>

using namespace Can::Model;
using namespace Can::Driver;
using namespace Can::Controller;

int main(int argc, char** argv) {
    MockDriver* driver = static_cast<MockDriver*>(DriverInterface::getInstance());
   
    Transmitter tx;

    constexpr uint16_t identifier = 1;
    uint64_t payload = 0x0123456789ABCDEF;

    if (0 < driver->txTrace.size()) {
        std::cerr << "TX trace not empty before message was sent!" << std::endl;
        return 1;
    }

    for (Can::Driver::MockDriver::FrameEvent& event : driver->txTrace) {
        if (identifier == event.identifier) {
            std::cerr << "Identifier is already in TX trace!" << std::endl;
            return 2;
        }
    }
    
    Message testMessage(
        identifier,
        &payload
    );

    tx.sendMessage(testMessage);

    if (0 == driver->txTrace.size()) {
        std::cerr << "No frame was sent!" << std::endl;
        return 3;
    } else if (1 < driver->txTrace.size()) {
        std::cerr << "More than one frame was sent!" << std::endl;
        return 4;
    }

    for (Can::Driver::MockDriver::FrameEvent& event : driver->txTrace) {
        if (identifier != event.identifier) {
            std::cerr << "Wrong frame identifier was sent!" << std::endl;
            return 5;
        }

        if (sizeof(payload) != event.payload.size()) {
            std::cerr << "Wrong payload size was sent!" << std::endl;
            return 6;
        }

        for (std::size_t byte = 0; byte < sizeof(payload); byte++) {
            const uint8_t expectedByte = static_cast<uint8_t>(
                payload >> ((sizeof(payload) - 1 - byte) * 8)
            );

            if (expectedByte != event.payload[byte]) {
                std::cerr << "Wrong payload byte at index " << byte
                          << "!" << std::endl;
                return 7;
            }
        }
    }

    return 0;
}