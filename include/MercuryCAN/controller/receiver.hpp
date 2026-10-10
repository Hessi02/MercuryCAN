/**
 * \file    receiver.hpp
 * \brief   Declares the CAN message receiver controller.
 *
 * The Receiver configures the driver to accept registered message identifiers
 * and forwards incoming frames to the matching cyclic messages. It keeps
 * non-owning references, so registered messages must outlive the Receiver.
 * For each match, it applies the received payload to the message's signal
 * data, sets its update flag, and emits a received notification with the
 * current millisecond tick count.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_CONTROLLER_RECEIVER_HPP__
#define __MERCURYCAN_CONTROLLER_RECEIVER_HPP__

#include <MercuryCAN/model/cyclicMessage.hpp>

#include <atomic>

namespace Can::Controller {

/**
 * \brief   Configures CAN message reception and processes received data.
 *
 * The Receiver registers non-owning references to cyclic messages with the
 * driver and applies incoming payloads to registered messages whose
 * identifiers match. It also provides a synchronous operation for waiting
 * until a Message's update flag is set.
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
     * identifier, and registers this Receiver with the driver. Incoming frames
     * matching the message are copied into it. The method blocks until the
     * complete receive callback has finished, then removes the identifier from
     * the driver's receive configuration.
     *
     * \warning This method has no timeout and can block indefinitely! 
     *
     * \param   message passes the message whose update flag is awaited.
     */
    void awaitMessage(Model::Message& message);

    /**
     * \brief   Registers a cyclic message for reception.
     *
     * Stores a non-owning reference to the message in the receiver's cyclic
     * message list, configures the driver to accept its identifier and payload
     * length, and registers this Receiver to process received frames. The
     * message must remain alive while it is registered.
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
     * \brief   Message currently awaited by awaitMessage(), if any.
     */
    std::atomic<Model::Message*> _awaitedMessage{nullptr};

    /**
     * \brief   Signals that the awaited frame has been fully processed.
     */
    std::atomic<bool> _awaitMessageReceived{false};

    /**
     * \brief   Number of cyclic messages registered with this receiver.
     */
    uint8_t _messageCount = 0;

    /**
     * \brief   Stored cyclic messages processed when matching frames arrive.
     */
    Generic::Container<Model::CyclicMessage*> _cyclicMessages;
};

}

#endif  //__MERCURYCAN_CONTROLLER_RECEIVER_HPP__