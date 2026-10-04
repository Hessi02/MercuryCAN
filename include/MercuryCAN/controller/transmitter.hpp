/**
 * \file    transmitter.hpp
 * \brief   Declares the CAN message transmitter controller.
 *
 * The Transmitter provides two transmission modes. Individual messages are
 * serialized and sent immediately when requested. Cyclic messages are
 * registered with the transmitter and checked during timer-driven processing;
 * A message is due when the current millisecond tick count is an exact
 * multiple of its non-zero cycle time.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__
#define __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__

#include <MercuryCAN/support/list/container.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

namespace Can::Controller {

/**
 * \brief   Sends CAN messages and schedules cyclic message transmissions.
 *
 * A Transmitter sends individual Message objects immediately when requested.
 * CyclicMessage objects can also be registered for periodic transmission.
 * The registered cyclic messages are held in shared static storage, and the
 * transmitter's timer-driven cycle processing checks their cycle times.
 *
 * Registered message objects must remain valid while they are used by the
 * transmitter.
 *
 * \see     Model::Message
 * \see     Model::CyclicMessage
 */
class Transmitter {
public:
    /**
     * \brief   Constructs a transmitter and activates the driver timer.
     *
     * The timer drives periodic processing of registered cyclic messages.
     */
    Transmitter(void);

    /**
     * \brief   Sends a message immediately.
     *
     * Serializes the message payload and passes its identifier and data to
     * the driver. After the transmit call returns, the message's sent
     * notification is emitted with the driver's current millisecond tick
     * count.
     *
     * \param   message passes the message to transmit.
     */
    void sendMessage(Model::Message& message) const;

    /**
     * \brief   Registers a message for cycle-based transmission.
     *
     * The message is stored as a non-owning pointer and the shared list is
     * sorted by CAN identifier. The message object must remain valid while it
     * is registered. Its preSend notification is emitted immediately before
     * each due transmission, and its sent notification is emitted after the
     * driver transmit call.
     *
     * \param   message passes the cyclic message to register.
     */
    void addCyclicMessage(Model::CyclicMessage& message);

    /**
     * \brief   Returns the number of registered cyclic messages.
     *
     * \return  Number of messages in the transmitter's shared cyclic list.
     */
    uint8_t getMessageCount(void) const;

    /**
     * \brief   Processes one periodic transmission cycle.
     *
     * Advances the driver's millisecond tick count, determines which
     * registered messages are due based on their cycle times, emits their
     * preSend notifications, transmits due messages, and then emits their
     * sent notifications. A message with a cycle time of zero is never due.
     *
     * This method is called by the driver's timer interrupt.
     */
    static void processTransmitCycle(void);

private:
    /**
     * \brief   Checks whether one registered message is due for transmission.
     *
     * For a non-zero cycle time, the message is due when the current tick
     * count is an exact multiple of that time. Marks due messages and emits
     * their preSend notification.
     *
     * \param   index passes the index of the message to check.
     * \param   tickCountMs passes the current driver tick count.
     * \param   dueMessages passes the array in which the due status is stored.
     */
    static void processMessage(
        const uint8_t& index,
        const uint32_t& tickCountMs,
        bool* dueMessages
    );

    /**
     * \brief   Transmits one message if it was marked as due.
     *
     * \param   index passes the index of the message to transmit.
     * \param   dueMessages passes the due-status array for this cycle.
     */
    static void transmitMessage(
        const uint8_t& index, const bool* dueMessages
    );

    /**
     * \brief   Emits the sent notification for a due message.
     *
     * \param   index passes the index of the message.
     * \param   tickCountMs passes the tick count associated with this cycle.
     * \param   dueMessages passes the due-status array for this cycle.
     */
    static void emitSentMessage(
        const uint8_t& index,
        const uint32_t& tickCountMs,
        const bool* dueMessages
    );
    
    /**
     * \brief   Number of messages registered in the shared cyclic list.
     */
    static inline uint8_t _messageCount = 0;

    /**
     * \brief   Shared non-owning pointers to registered cyclic messages.
     */
    static inline Generic::Container<Model::CyclicMessage*> _cyclicMessages;
};

}

#endif  //__MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__
