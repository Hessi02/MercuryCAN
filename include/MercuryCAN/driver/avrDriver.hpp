#ifndef __MERCURYCAN_CONTROLLER_AVR_DRIVER_HPP__
#define __MERCURYCAN_CONTROLLER_AVR_DRIVER_HPP__

#include <MercuryCAN/driver/driver.hpp>

#include <cstdlib>
#include <stdint.h>

namespace Can::Controller {

class Receiver;

class AvrDriver : public Driver
{
public:
    friend Driver;

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

private:
    void configureTransmitMessage(
        const uint8_t& messageObject,
        const uint16_t& identifier
    ) const;

    void transmitPayload(
        const uint8_t* data, const std::size_t& payloadLength
    ) const;

    void completeTransmission(const uint8_t& messageObject);

    void initHardware(void) const;

    uint8_t reserveMessageObject(void);

    void freeMessageObject(const uint8_t& index);

    void resetMessageObjects(void) const;

    AvrDriver(void);

    Receiver* _receiver = nullptr;

    uint32_t _tickCountMs = 0;
    static inline constexpr uint8_t _messageObjectCount = 15;
    static inline uint16_t _usedMessageObjectMask = 0;
};

}

#endif  // __MERCURYCAN_CONTROLLER_AVR_DRIVER_HPP__
