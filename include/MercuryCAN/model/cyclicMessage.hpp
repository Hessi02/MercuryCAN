/**
 * \file    cyclicMessage.hpp
 * \brief   Defines the CAN cyclic message model.
 *
 * A CyclicMessage extends the Message model with a cycle time in milliseconds.
 * The cycle time can be used by a controller to determine when the message is
 * due for transmission.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_MODEL_CYCLIC_MESSAGE_HPP__
#define __MERCURYCAN_MODEL_CYCLIC_MESSAGE_HPP__

#include <MercuryCAN/model/message.hpp>

namespace Can::Model {

/**
 * \brief   Represents a CAN message with an associated cycle time.
 *
 * A CyclicMessage extends Message with a cycle time in milliseconds. It
 * inherits the CAN identifier, signal references, payload handling, and
 * notification callbacks from Message.
 *
 * \see     Message
 */
class CyclicMessage : public Message {
public:
    /**
     * \brief   Constructs object from identifier, cycle time and signals.
     *
     * The identifier and signal references are passed to the Message
     * constructor. The referenced signal data objects must remain valid for
     * the lifetime of the CyclicMessage.
     *
     * \tparam  SignalDataTypes passes the types of the signal data.
     * \param   identifier passes the CAN identifier for this message.
     * \param   cycleTime passes the message cycle time in milliseconds.
     * \param   signalReferences passes pointers to the signal data objects.
     *
     * \see     AllowedSignalDataType
     * \see     FitsIntoCanMessage
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes>, ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    CyclicMessage(
        const uint16_t& identifier,
        const uint16_t& cycleTime,
        SignalDataTypes*... signalReferences
    )
        : Message(identifier, signalReferences...), _cycleTime(cycleTime) {
    }

    /**
     * \brief   Returns the cycle time of this message.
     *
     * \return  Cycle time in milliseconds supplied to the constructor.
     */
    uint16_t getCycleTime(void) const {
        return _cycleTime;
    }

private:
    /**
     * \brief   Cycle time of this message in milliseconds.
     */
    const uint16_t _cycleTime;
};

}

#endif  // __MERCURYCAN_MODEL_CYCLIC_MESSAGE_HPP__