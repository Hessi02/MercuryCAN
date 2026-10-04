/**
 * \file    receiver.hpp
 * \brief   Declares the CAN message receiver controller.
 *
 * The Receiver configures the driver to accept registered message identifiers
 * and forwards incoming frames to the matching cyclic messages. For each
 * match, it applies the received payload to the message's signal data, sets
 * its update flag, and emits a received notification with the current
 * millisecond tick count.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_CONTROLLER_RECEIVER_HPP__
#define __MERCURYCAN_CONTROLLER_RECEIVER_HPP__

#include <MercuryCAN/model/cyclicMessage.hpp>

namespace Can::Controller {

/**
 * \brief   Configures CAN message reception and processes received data.
 *
 * The Receiver registers cyclic messages with the driver and applies incoming
 * payloads to registered messages whose identifiers match. It also provides
 * a synchronous operation for waiting until a Message's update flag is set.
 *
 * \see     Model::Message
 * \see     Model::CyclicMessage
 */
class Receiver {
public:
    /**
     * \brief   Waits synchronously for a message update.
     *
     * Clears the message's update flag, configures the driver to receive its
     * identifier, and registers this Receiver with the driver. The method
     * blocks until the message's update flag becomes true, then removes that
     * identifier from the driver's receive configuration.
     *
     * \warning This method has no timeout and can block indefinitely! 
     *
     * \param   message passes the message whose update flag is awaited.
     */
    void awaitMessage(Model::Message& message);

    /**
     * \brief   Registers a cyclic message for reception.
     *
     * Stores the message in the receiver's cyclic message list, configures the
     * driver to accept its identifier and payload length, and registers this
     * Receiver to process received frames.
     *
     * \param   message passes the cyclic message to register.
     */
    void addCyclicMessage(Model::CyclicMessage& message);

    /**
     * \brief   Applies received payload data to matching registered messages.
     *
     * For each registered cyclic message with an identifier matching
     * \p identifier, applies the available payload bytes, sets its update
     * flag, and emits its received notification with the driver's current
     * millisecond tick count. Payload processing follows the behavior of
     * Message::setPayloadData().
     *
     * \param   identifier passes the CAN identifier of the received frame.
     * \param   data passes the received payload bytes.
     * \param   dataLength passes the number of available payload bytes.
     */
    void processRxData(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& dataLength
    );

private:
    /**
     * \brief   Number of cyclic messages registered with this receiver.
     */
    uint8_t _messageCount = 0;

    /**
     * \brief   Stored cyclic messages processed when matching frames arrive.
     */
    Generic::Container<Model::CyclicMessage> _cyclicMessages;
};

}

#endif  //__MERCURYCAN_CONTROLLER_RECEIVER_HPP__