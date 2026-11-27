/**
 * \file    receiver.hpp
 * \brief   Declares the CAN frame receiver controller.
 *
 * The Receiver configures the driver to accept registered frame identifiers and 
 * forwards incoming frames to the matching cyclic frames. It keeps non-owning 
 * references, so registered frames must outlive the Receiver. For each match, 
 * it applies the received payload to the frame's signal data, sets its update 
 * flag, and emits a received notification with the current millisecond tick 
 * count.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_CONTROLLER_RECEIVER_HPP__
#define __MERCURYCAN_CONTROLLER_RECEIVER_HPP__

#include <MercuryCAN/model/runtime/cyclicFrame.hpp>

#include <vector>

namespace Can::Controller 
{

/**
 * \brief   Configures CAN frame reception and processes received data.
 *
 * The Receiver registers non-owning references to cyclic frames with the driver 
 * and applies incoming payloads to registered frames whose identifiers match. 
 * It also provides a synchronous operation for waiting until a Frame's update 
 * flag is set.
 */
class Receiver 
{
public:
    /**
     * \brief   Waits synchronously for a frame update.
     *
     * Clears the frame's update flag, configures the driver to receive its
     * identifier, and registers this Receiver with the driver. Incoming frames
     * matching the frame are copied into it. The method blocks until the
     * complete receive callback has finished, then removes the identifier from
     * the driver's receive configuration.
     *
     * \warning This method has no timeout and can block indefinitely! 
     *
     * \param   frame passes the frame whose update flag is awaited.
     */
    void awaitMessage(Model::Runtime::Frame& frame);

    /**
     * \brief   Waits synchronously for a frame update until timeout expired.
     *
     * Overloaded awaitMessage() method, returning with false when the frame
     * does not get updated until timeout expired. The method returns with true,
     * if the frame is received in time. The driver's millisecond timer is
     * activated when the timeout is nonzero.
     *
     * \param   frame passes the frame whose update flag is awaited.
     * \param   timeout passes the duration to wait for a frame update in ms.
     */
    bool awaitMessage(Model::Runtime::Frame& frame, const uint32_t& timeout);

    /**
     * \brief   Registers a cyclic frame for reception.
     *
     * Stores a non-owning reference to the frame in the receiver's cyclic frame 
     * list, configures the driver to accept its identifier and payload length, 
     * and registers this Receiver to process received frames. The frame must 
     * remain alive while it is registered.
     *
     * \param   frame passes the cyclic frame to register.
     */
    void addCyclicMessage(Model::Runtime::CyclicFrame& frame);

    /**
     * \brief   Applies received payload data to matching registered frames.
     *
     * For each registered cyclic frame with a matching identifier, applies the 
     * available payload bytes, sets its update flag, and emits its received 
     * notification with the driver's current millisecond tick count. Payload 
     * processing follows the behavior of Frame::setPayloadData().
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
     * \brief   Frame currently awaited by awaitMessage(), if any.
     */
    Model::Runtime::Frame* volatile _awaitedMessage = nullptr;

    /**
     * \brief   Signals that the awaited frame has been fully processed.
     */
    volatile bool _awaitMessageReceived = false;

    /**
     * \brief   Stored cyclic frames processed when matching frames arrive.
     */
    std::vector<Model::Runtime::CyclicFrame*> _cyclicMessages;
};

}

#endif  //__MERCURYCAN_CONTROLLER_RECEIVER_HPP__
