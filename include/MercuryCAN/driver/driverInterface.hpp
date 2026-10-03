/**
 * \file    driverInterface.hpp
 * \brief   Defines the abstract CAN driver interface.
 *
 * The DriverInterface provides the CAN hardware operations used by the
 * controllers, including message transmission and reception, receive-message
 * registration, and millisecond tick counting. A concrete driver
 * implementation is obtained through getInstance().
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_DRIVER_INTERFACE_HPP__
#define __MERCURYCAN_DRIVER_INTERFACE_HPP__

#include <cstdlib>
#include <stdint.h>

namespace Can::Controller {

class Receiver;

}

namespace Can::Driver {

/**
 * \brief   Defines the interface between the CAN controllers and the driver.
 *
 * DriverInterface provides the hardware-facing operations required by the
 * transmitter and receiver controllers. Implementations provide the actual
 * CAN communication, timer, and power-management behavior. The interface does
 * not take ownership of payload buffers or the Receiver instance passed to it.
 */
class DriverInterface 
{
public:
    /**
     * \brief   Returns the driver instance used by the controllers.
     *
     * Controllers use this function to access the configured concrete driver
     * without depending directly on its implementation type.
     *
     * \return  Pointer to the driver instance.
     */
    static DriverInterface* getInstance(void);

    /**
     * \brief   Sends a CAN frame with the given identifier and payload.
     *
     * The implementation uses the supplied payload bytes for transmission.
     * The caller retains ownership of \p data and must keep the buffer valid
     * for the duration of this call.
     *
     * \param   identifier passes the CAN identifier of the message.
     * \param   data passes the address of the payload buffer.
     * \param   payloadLength passes the number of payload bytes to transmit.
     */
    virtual void transmit(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) = 0;

    /**
     * \brief   Forwards received CAN frame data to the registered receiver.
     *
     * Drivers call this method when a frame is received. If a Receiver
     * instance has been registered, the frame data is passed to it for
     * processing. The payload buffer is borrowed; its ownership remains with
     * the caller, and the receiver must not retain the pointer beyond the
     * call unless it copies the data.
     *
     * \param   identifier passes the CAN identifier of the message.
     * \param   data passes the address of the received payload buffer.
     * \param   payloadLength passes the number of received payload bytes.
     */
    virtual void receive(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) = 0;

    /**
     * \brief   Configures reception for a CAN message.
     *
     * The driver configures its receive hardware to accept frames matching
     * \p identifier and the specified payload length. Received frames are
     * forwarded through receive() to the registered Receiver.
     *
     * \param   identifier passes the CAN identifier to accept.
     * \param   length passes the expected payload length in bytes.
     */
    virtual void addRxMessage(
        const uint16_t& identifier, 
        const uint8_t& length
    ) = 0;

    /**
     * \brief   Places the system in the driver's idle sleep mode.
     *
     * The concrete driver determines the sleep behavior and the events that
     * can wake the system.
     */
    virtual void enterIdleSleep(void) const = 0;

    /**
     * \brief   Stops reception for the specified CAN identifier.
     *
     * After removal, the driver no longer keeps the receive configuration for
     * \p identifier active.
     *
     * \param   identifier passes the CAN identifier to remove.
     */
    virtual void removeRxMessage(const uint16_t& identifier) = 0;

    /**
     * \brief   Starts the timer used to process cyclic transmissions.
     *
     * The timer periodically drives the transmitter's cycle processing and
     * advances the millisecond tick count used by message scheduling and
     * notification callbacks.
     */
    virtual void activateTxTimer(void) = 0;

    /**
     * \brief   Registers the receiver that handles incoming CAN messages.
     *
     * The driver stores the supplied pointer and uses it to forward received
     * frames. The pointer is non-owning; the Receiver must remain valid for as
     * long as the driver may use it. Passing \c nullptr clears the registered
     * receiver.
     *
     * \param   recv passes a pointer to the receiver instance.
     */
    virtual void setReceiverInstance(Controller::Receiver* recv) = 0;

    /**
     * \brief   Returns the driver's current millisecond tick count.
     *
     * The count is used as an elapsed-time reference for cyclic message
     * scheduling and callbacks. Its initial value and wraparound behavior are
     * determined by the concrete driver implementation.
     *
     * \return  Current tick count in milliseconds.
     */
    virtual uint32_t getTickCountMs(void) const = 0;

    /**
     * \brief   Advances the driver's millisecond tick count by one.
     *
     * This operation is called by the transmitter's periodic cycle
     * processing. The concrete driver is responsible for ensuring that the
     * timer period corresponds to one millisecond.
     */
    virtual void incrementTickCountMs(void) = 0;
};

}

#endif  //__MERCURYCAN_DRIVER_INTERFACE_HPP__
