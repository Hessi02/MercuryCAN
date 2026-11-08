/**
 * \file    avrDriver.hpp
 * \brief   Declares the AVR implementation of the CAN driver interface.
 *
 * AvrDriver provides CAN communication, receive-message configuration, idle
 * sleep, and millisecond timer support using the AVR CAN controller hardware.
 * It implements DriverInterface and is created by it, not directly by callers.
 * Additional public members would require callers to cast to AvrDriver, which
 * is not expected. Keep this class simple; add only private implementation
 * details.
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

namespace Can::Controller 
{

class Receiver;

}

namespace Can::Driver 
{

/**
 * \brief   Implements the CAN driver interface using AVR CAN hardware.
 *
 * AvrDriver configures the AVR CAN controller, allocates its message objects
 * for transmission and reception, and forwards received frames to a Receiver. 
 * The instance is created by DriverInterface and is not constructed directly by 
 * callers. This specific AVR implementation uses the Timer0 in clear-on-compare 
 * mode with a prescaler and the CAN controller with 15 message objects.
 * 
 * \warning The Timer0 can't be used for other purposes in user code anymore!
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
     * message object. This method blocks until the frame is sent. If data is
     * null or no message object is available, it returns without transmitting.
     *
     * \todo    Consider a non-blocking implementation that uses a queue.
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
     * is ignored. While the transmit method can be viewed as the controller's
     * output, this method is the controller's input. 
     *
     * \param   identifier passes the CAN identifier of the received frame.
     * \param   data passes the received payload bytes.
     * \param   payloadLength passes the number of received payload bytes.
     */
    virtual void forwardReceiveFrame(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    /**
     * \brief   Configures an available message object to receive a CAN frame.
     *
     * The object is configured to match identifier and accept the specified
     * payload length. Its receive interrupt is enabled. If no message object
     * is available, no receive configuration is added. A corresponding method
     * to remove the receive configuration is also provided.
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
     * This method does not return. Although this is the weakest sleep mode, the 
     * CPU is stopped and the power consumption is reduced. Any deeper modes 
     * would disable the CAN controller and its interrupts. For a deeper sleep
     * an interrupt pin could be connected to a CAN transceiver's wakeup output, 
     * which is not implemented here.
     */
    virtual void enterIdleSleep(void) const override final;

    /**
     * \brief   Releases receive message objects matching a CAN identifier.
     *
     * Matching receive objects are freed and their receive interrupts disabled. 
     * The method ignores identifiers that are not configured for reception. Any
     * message objects that are not used for reception or don't match the passed
     * identifier are not affected. The method does not return any status to the
     * caller, as it is assumed that the caller does not need to know whether a
     * message object was actually released or not.
     *
     * \param   identifier passes the CAN identifier to remove.
     */
    virtual void removeRxMessage(const uint16_t& identifier) override final;
    
    /**
     * \brief   Enables the timer interrupt for cyclic transmission processing.
     *
     * Configures Timer0 in clear-on-compare mode with a compare value and
     * prescaler intended to generate a one-millisecond period at the target
     * clock frequency, then enables a compare interrupt. The interrupt service 
     * routine calls the static processTransmitCycle() method to process cyclic 
     * messages. 
     */
    virtual void activateTxTimer(void) override final;

    /**
     * \brief   Sets the receiver used to process incoming CAN frames.
     *
     * The receiver pointer is non-owning. The receiver must remain valid for
     * as long as this driver may forward frames to it. Only one receiver can be 
     * registered at a time. Passing a null pointer clears the receiver, and any 
     * received frames are ignored. The user should always only use one receiver 
     * instance, as the driver does not support multiple receivers.
     * 
     * \todo    Move this implementation to the base class.
     *
     * \param   recv passes the receiver instance, or nullptr to clear it.
     */
    virtual void setReceiverInstance(Controller::Receiver* recv) override final;

    /**
     * \brief   Returns the driver's current millisecond tick count.
     *
     * The count is used as an elapsed-time reference for cyclic message
     * scheduling and callbacks. Its initial value and wraparound behavior are
     * determined by the concrete driver implementation. 
     *
     * \return  Current value of the driver's tick counter.
     */
    virtual uint32_t getTickCountMs(void) const override final;

    /**
     * \brief   Increments the driver's millisecond tick counter.
     * 
     * This operation is called by the transmitter's periodic cycle processing. 
     * The tick count should therefore mirror the hardware timer's value.
     * Ideally this method would not be public, but the Transmitter needs access
     * to it when processing timer interrupts. Application code should not call
     * it.
     */
    virtual void incrementTickCountMs(void) override final;

private:
    /**
     * \brief   Configures a message object for CAN transmission.
     *
     * Selects the message object and writes the identifier to the controller 
     * registers, with no identifier mask applied. The identifier mask is not
     * used for transmission, as the controller does not filter outgoing frames.
     * A message object must be reserved before calling this method.
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
     * The payload is written to the controller's message object registers. At
     * this point, the target message object must have been selected and
     * configured with the identifier. The caller must ensure that the payload
     * length does not exceed the maximum allowed by the CAN standard. The
     * transmission is started by setting the transmit request bit in the
     * message object control register, but this method does not wait for
     * completion.
     * 
     * \param   data passes the payload bytes to write.
     * \param   payloadLength passes the number of payload bytes.
     */
    void transmitPayload(
        const uint8_t* data, 
        const std::size_t& payloadLength
    ) const;

    /**
     * \brief   Waits for complete transmission and releases its message object.
     * 
     * The method waits for the transmit complete flag to be set in the message
     * object status register, then clears the status and control registers and
     * releases the message object. The method blocks until the transmission is
     * complete, which may take several milliseconds. The caller must ensure the
     * message object is still reserved and configured for transmission when it 
     * is called.
     *
     * \param   messageObject passes the index of the message object.
     */
    void completeTransmission(const uint8_t& messageObject);

    /**
     * \brief   Resets and enables the AVR CAN hardware.
     *
     * Configures the CAN bit timing, clears message objects, enables CAN
     * interrupts, and enables global interrupts. The method is called by the
     * constructor and should not be called again. The CAN controller is reset
     * and reconfigured, which may disrupt any ongoing communication.
     */
    void initHardware(void) const;

    /**
     * \brief   Reserves the first unused CAN message object.
     * 
     * The method manages the bitflag representation of used message objects. If 
     * all message objects are in use, it returns an out-of-bounds index. The 
     * caller must ensure that the returned index is valid before using it to
     * access the controller's message object registers.
     *
     * \return  Index of the reserved object or out-of-bounds if all in use.
     */
    uint8_t reserveMessageObject(void);

    /**
     * \brief   Marks a CAN message object as available.
     *
     * The method clears the corresponding bit in the bitflag representation of 
     * used message objects. If the index is out-of-bounds, it is ignored. The
     * caller must ensure that the message object is no longer in use before
     * calling this method, as it does not check the controller's registers and
     * status.
     *
     * \param   index passes the message object index to release.
     */
    void freeMessageObject(const uint8_t& index);

    /**
     * \brief   Clears the configuration and status of every CAN message object.
     * 
     * The method iterates through all message objects, clears their control and
     * status registers, and resets their identifier and mask registers. The
     * message objects are not reserved or released; the caller must ensure that
     * no message object is in use when this method is called. The method does
     * not disable interrupts or change the CAN controller's operating mode.
     * It is intended to be called during initialization, when the controller is
     * reset and no message objects are in use.
     */
    void resetMessageObjects(void) const;

    /**
     * \brief   Constructs the AVR driver and initializes the CAN hardware.
     *
     * The constructor is private and called by DriverInterface. It initializes
     * the CAN controller, resets message objects, and sets the tick count to
     * zero. The driver instance is not constructed directly by callers, and the
     * constructor should not be called again after the instance is created.
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
