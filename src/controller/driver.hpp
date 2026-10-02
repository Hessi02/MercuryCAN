#ifndef __CAN_CONTROLLER_DRIVER_HPP__
#define __CAN_CONTROLLER_DRIVER_HPP__

#include <cstdlib>
#include <stdint.h>

namespace Can::Controller {

class Receiver;

class Driver 
{
public:
    static Driver* getInstance(void);

    virtual void transmit(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) = 0;

    virtual void receive(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) = 0;

    virtual void addRxMessage(
        const uint16_t& identifier, 
        const uint8_t& length
    ) = 0;

    virtual void enterIdleSleep(void) const = 0;

    virtual void removeRxMessage(const uint16_t& identifier) = 0;

    virtual void activateTxTimer(void) = 0;

    virtual void setReceiverInstance(Receiver* recv) = 0;

    virtual uint32_t getTickCountMs(void) const = 0;

    virtual void incrementTickCountMs(void) = 0;
};

}

#endif  //__CAN_CONTROLLER_DRIVER_HPP__
