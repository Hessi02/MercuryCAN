/**
 * \file    avrDriver.hpp
 * \brief   Declares the AVR implementation of the CAN driver interface.
 *
 * AvrDriver provides CAN communication, receive-message configuration, idle 
 * sleep, and millisecond timer support using the AVR CAN controller hardware.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_DRIVER_AVR_DRIVER_HPP__
#define __MERCURYCAN_DRIVER_AVR_DRIVER_HPP__

#include <MercuryCAN/driver/driverInterface.hpp>

#include <cstdlib>
#include <stdint.h>

namespace Can::Controller {

class Receiver;

}

namespace Can::Driver {

/**
 * \brief   Implements the CAN driver interface using AVR CAN hardware.
 *
 * AvrDriver configures the AVR CAN controller, allocates its message objects
 * for transmission and reception, and forwards received frames to a
 * Controller::Receiver. The instance is created by DriverInterface and is
 * not constructed directly by callers.
 *
 * \see     DriverInterface
 */
class AvrDriver : public DriverInterface
{
public:
    friend DriverInterface;

    /**
     * \brief   Transmits a CAN frame using an available message object.
     *
     * Configures a message object with the supplied identifier, writes the
     * payload, waits for transmission completion, and then releases the
     * message object. If \p data is null or no message object is available,
     * the method returns without transmitting.
     *
     * \param   identifier passes the CAN identifier of the frame.
     * \param   data passes the payload bytes to transmit.
     * \param   payloadLength passes the number of payload bytes.
     */
    virtual void transmit(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    /**
     * \brief   Forwards a received frame to the registered receiver.
     *
     * If a receiver has been registered, its processRxData() method is called
     * with the identifier and payload. If no receiver is registered, the frame
     * is ignored.
     *
     * \param   identifier passes the CAN identifier of the received frame.
     * \param   data passes the received payload bytes.
     * \param   payloadLength passes the number of received payload bytes.
     */
    virtual void receive(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    /**
     * \brief   Configures an available message object to receive a CAN frame.
     *
     * The object is configured to match \p identifier and accept the specified
     * payload length. Its receive interrupt is enabled. If no message object
     * is available, no receive configuration is added.
     *
     * \param   identifier passes the CAN identifier to accept.
     * \param   length passes the expected payload length in bytes.
     */
    virtual void addRxMessage(
        const uint16_t& identifier, 
        const uint8_t& length
    ) override final;

    /**
     * \brief   Repeatedly enters AVR idle sleep mode.
     *
     * The CPU sleeps between interrupts and resumes to return to sleep again.
     * This method does not return.
     */
    virtual void enterIdleSleep(void) const override final;

    /**
     * \brief   Releases receive message objects matching a CAN identifier.
     *
     * Matching receive objects are freed and their receive interrupts are
     * disabled.
     *
     * \param   identifier passes the CAN identifier to remove.
     */
    virtual void removeRxMessage(const uint16_t& identifier) override final;
    
    /**
     * \brief   Enables the timer interrupt for cyclic transmission processing.
     *
     * Configures Timer0 in clear-on-compare mode with a compare value and
     * prescaler intended to generate a one-millisecond period at the target
     * clock frequency, then enables its compare interrupt.
     */
    virtual void activateTxTimer(void) override final;

    /**
     * \brief   Sets the receiver used to process incoming CAN frames.
     *
     * The receiver pointer is non-owning. The receiver must remain valid for
     * as long as this driver may forward frames to it.
     *
     * \param   recv passes the receiver instance, or \c nullptr to clear it.
     */
    virtual void setReceiverInstance(Controller::Receiver* recv) override final;

    /**
     * \brief   Returns the driver's current millisecond tick count.
     *
     * \return  Current value of the driver's tick counter.
     */
    virtual uint32_t getTickCountMs(void) const override final;

    /**
     * \brief   Increments the driver's millisecond tick counter.
     */
    virtual void incrementTickCountMs(void) override final;

private:
    /**
     * \brief   Configures a message object for CAN transmission.
     *
     * Selects the message object and writes the identifier to the CAN
     * controller registers, with no identifier mask applied.
     *
     * \param   messageObject passes the index of the message object.
     * \param   identifier passes the CAN identifier to transmit.
     */
    void configureTransmitMessage(
        const uint8_t& messageObject,
        const uint16_t& identifier
    ) const;

    /**
     * \brief   Writes the payload and starts CAN transmission.
     *
     * \param   data passes the payload bytes to write.
     * \param   payloadLength passes the number of payload bytes.
     */
    void transmitPayload(
        const uint8_t* data, const std::size_t& payloadLength
    ) const;

    /**
     * \brief   Waits for transmission completion and releases its message object.
     *
     * \param   messageObject passes the index of the transmitted message object.
     */
    void completeTransmission(const uint8_t& messageObject);

    /**
     * \brief   Resets and enables the AVR CAN hardware.
     *
     * Configures the CAN bit timing, clears message objects, enables CAN
     * interrupts, and enables global interrupts.
     */
    void initHardware(void) const;

    /**
     * \brief   Reserves the first unused CAN message object.
     *
     * \return  Index of the reserved object, or _messageObjectCount if all
     *          objects are in use.
     */
    uint8_t reserveMessageObject(void);

    /**
     * \brief   Marks a CAN message object as available.
     *
     * Indices outside the supported message-object range are ignored.
     *
     * \param   index passes the message object index to release.
     */
    void freeMessageObject(const uint8_t& index);

    /**
     * \brief   Clears the configuration and status of every CAN message object.
     */
    void resetMessageObjects(void) const;

    /**
     * \brief   Constructs the AVR driver and initializes the CAN hardware.
     *
     * This constructor is private; DriverInterface creates the driver instance.
     */
    AvrDriver(void);

    /**
     * \brief   Non-owning pointer to the receiver for incoming frames.
     */
    Controller::Receiver* _receiver = nullptr;

    /**
     * \brief   Number of elapsed timer ticks, counted in milliseconds.
     */
    uint32_t _tickCountMs = 0;

    /**
     * \brief   Number of message objects available in the AVR CAN controller.
     */
    static inline constexpr uint8_t _messageObjectCount = 15;

    /**
     * \brief   Bit mask tracking which CAN message objects are in use.
     */
    static inline uint16_t _usedMessageObjectMask = 0;
};

}

#endif  //__MERCURYCAN_DRIVER_AVR_DRIVER_HPP__
