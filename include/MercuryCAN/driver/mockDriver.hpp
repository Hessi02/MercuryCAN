#ifndef __MERCURYCAN_CONTROLLER_MOCK_DRIVER_HPP__
#define __MERCURYCAN_CONTROLLER_MOCK_DRIVER_HPP__

#include <MercuryCAN/driver/driverInterface.hpp>

#include <cstdlib>
#include <stdint.h>

namespace Can::Controller {

class Receiver;

class MockDriver : public DriverInterface
{
public:
    friend DriverInterface;

    virtual void transmit(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    virtual void receive(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    virtual void addRxMessage(
        const uint16_t& identifier, 
        const uint8_t& length
    ) override final;

    virtual void enterIdleSleep(void) const override final;

    virtual void removeRxMessage(const uint16_t& identifier) override final;
    
    virtual void activateTxTimer(void) override final;

    virtual void setReceiverInstance(Receiver* recv) override final;

    virtual uint32_t getTickCountMs(void) const override final;

    virtual void incrementTickCountMs(void) override final;
};

}

#endif  // __MERCURYCAN_CONTROLLER_MOCK_DRIVER_HPP__
