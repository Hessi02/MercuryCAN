/**
 * \file    transmitter.hpp
 * \brief   Declares the CAN frame transmitter controller.
 *
 * The Transmitter provides two transmission modes. Individual frames are
 * serialized and sent immediately when requested. Cyclic frames are registered 
 * with the transmitter and checked during timer-driven processing; A frame is 
 * due when the current millisecond tick count is an exact multiple of its 
 * non-zero cycle time.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__
#define __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__

#include <MercuryCAN/model/runtime/cyclicFrame.hpp>

#include <vector>

namespace Can::Controller 
{

/**
 * \brief   Sends CAN frames and schedules cyclic frame transmissions.
 *
 * A Transmitter sends individual Frame objects immediately when requested.
 * CyclicFrame objects can also be registered for periodic transmission. The 
 * registered cyclic frames are held in shared static storage, and the
 * transmitter's timer-driven cycle processing checks their cycle times.
 *
 * \warning Registered frame objects must remain valid while they are used!
 */
class Transmitter 
{
public:
    /**
     * \brief   Constructs a transmitter and activates the driver timer.
     *
     * The timer drives periodic processing of registered cyclic frames. Its
     * implementation is driver-specific and may be a hard- or software timer. 
     */
    Transmitter(void);

    /**
     * \brief   Sends a frame immediately.
     *
     * Serializes the frame payload and passes its identifier and data to the 
     * driver. After the transmit call returns, the frame's sent notification is 
     * emitted with the driver's current tick count.
     *
     * \param   frame passes the frame to transmit.
     */
    void sendMessage(Model::Runtime::Frame& frame) const;

    /**
     * \brief   Registers a frame for cycle-based transmission.
     *
     * The frame is stored as a non-owning pointer and the shared list is sorted 
     * by CAN identifier. The frame object must remain valid while it is 
     * registered. Its preSend notification is emitted immediately before each 
     * due transmission, and its sent notification is emitted after the driver 
     * transmit call.
     * 
     * \note    The 'preSend' signal is more timing-critical than 'sent'.
     *
     * \param   frame passes the cyclic frame to register.
     */
    void addCyclicMessage(Model::Runtime::CyclicFrame& frame);

    /**
     * \brief   Returns the number of registered cyclic frames.
     *
     * This reports the number of registered frames; the transmitter does not
     * impose a maximum. A driver may be unable to transmit a frame when no 
     * frame object is available.
     * 
     * \return  Number of frames in the transmitter's shared cyclic vector.
     */
    std::size_t getMessageCount(void) const;

    /**
     * \brief   Processes one periodic transmission cycle.
     *
     * Advances the driver's millisecond tick count, determines which registered 
     * frames are due based on their cycle times, emits their preSend signals,
     * transmits due frames and then emits their sent notifications. A frame
     * with a cycle time of zero is never due.
     *
     * \note This method is called by the driver's timer interrupt.
     */
    static void processTransmitCycle(void);

private:
    /**
     * \brief   Checks whether one registered frame is due for transmission.
     *
     * For a non-zero cycle time, the frame is due when the current tick
     * count is an exact multiple of that time. Marks due frames and emits
     * their preSend notification.
     *
     * \param   index passes the index of the frame to check.
     * \param   tickCountMs passes the current driver tick count.
     * \param   dueMessages passes the array in which the due status is stored.
     */
    static void processMessage(
        const std::size_t& index,
        const uint32_t& tickCountMs,
        std::vector<bool>& dueMessages
    );

    /**
     * \brief   Transmits one frame if it was marked as due.
     *
     * The frame's identifier and serialized payload are passed to the driver.
     * It is assumed that the driver will not block and will return immediately. 
     * Mind that the frame at the given index may not be due for transmission in 
     * this cycle. In that case, the driver is not called and the frame is
     * skipped.
     * 
     * \param   index passes the index of the frame to transmit.
     * \param   dueMessages passes the due-status array for this cycle.
     */
    static void transmitMessage(
        const std::size_t& index,
        const std::vector<bool>& dueMessages
    );

    /**
     * \brief   Emits the sent notification for a due frame.
     *
     * The frame's sent signal is emitted with the current tick count. It is
     * assumed that the driver has already transmitted the frame. The usage is
     * preferred over the 'preSend' signal, which is more timing-critical and 
     * may delay the frame's transmission if the callback takes too long to
     * execute.
     * 
     * \param   index passes the index of the frame.
     * \param   tickCountMs passes the tick count associated with this cycle.
     * \param   dueMessages passes the due-status array for this cycle.
     */
    static void emitSentMessage(
        const std::size_t& index,
        const uint32_t& tickCountMs,
        const std::vector<bool>& dueMessages
    );
    
    /**
     * \brief   Shared non-owning pointers to registered cyclic frames.
     */
    static inline std::vector<Model::Runtime::CyclicFrame*> _cyclicMessages;

    /**
     * \brief   Per-cycle transmission flags, resized when frames are added.
     */
    static inline std::vector<bool> _dueMessages;
};

}

#endif  //__MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__
