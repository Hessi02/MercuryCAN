/**
 * \file    cyclicFrame.hpp
 * \brief   Defines the CAN cyclic frame model.
 *
 * A CyclicFrame extends the Frame model with a cycle time in milliseconds. The 
 * cycle time can be used by a controller to determine when the frame is due for 
 * transmission.  
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_MODEL_RUNTIME_CYCLIC_FRAME_HPP__
#define __MERCURYCAN_MODEL_RUNTIME_CYCLIC_FRAME_HPP__

#include <MercuryCAN/model/runtime/frame.hpp>

namespace Can::Model::Runtime 
{

/**
 * \brief   Represents a CAN frame with an associated cycle time.
 *
 * A CyclicFrame extends Frame with a cycle time in milliseconds. It inherits 
 * the CAN identifier, signal references, payload handling, and notification 
 * callbacks from Frame. 
 */
class CyclicFrame : public Frame 
{
public:
    /**
     * \brief   Constructs object from identifier, cycle time and signals.
     *
     * The identifier and signal references are passed to the Frame constructor. 
     * The referenced signal data objects must remain valid for the lifetime of 
     * the CyclicFrame.
     *
     * \tparam  SignalDataTypes passes the types of the signal data.
     * \param   identifier passes the CAN identifier for this frame.
     * \param   cycleTime passes the frame cycle time in milliseconds.
     * \param   signalReferences passes pointers to the signal data objects.
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanFrame<SignalDataTypes...>
    constexpr CyclicFrame(
        const uint16_t& identifier,
        const uint16_t& cycleTime,
        SignalDataTypes*... signalReferences
    )
        : Frame(identifier, signalReferences...), _cycleTime(cycleTime) {
    }

    /**
     * \brief   Returns the cycle time of this frame.
     *
     * The cycle time is the interval in milliseconds at which the frame is
     * expected to be transmitted. It can be used by a controller to determine
     * when the frame is due for transmission.
     * 
     * \return  Cycle time in milliseconds supplied to the constructor.
     */
    constexpr uint16_t getCycleTime(void) const {
        return _cycleTime;
    }

private:
    /**
     * \brief   Cycle time of this frame in milliseconds.
     */
    const uint16_t _cycleTime;
};

}

#endif  //__MERCURYCAN_MODEL_RUNTIME_CYCLIC_FRAME_HPP__
