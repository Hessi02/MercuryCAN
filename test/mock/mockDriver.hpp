/**
 * \file    mockDriver.hpp
 * \brief   Mock driver for testing the MercuryCAN library.
 * 
 * The mock driver simulates the behavior of a real CAN driver, allowing for 
 * unit testing of the MercuryCAN library without requiring actual hardware. It
 * provides a controlled environment to test various scenarios, including 
 * message transmission, reception, and error handling. Controller classes can
 * use this mock driver to verify their functionality and ensure that the 
 * library behaves as expected under different conditions.
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 * 
 * SPDX-License-Identifier: Apache-2.0 
 */

#ifndef __MOCK_MOCK_DRIVER_HPP__
#define __MOCK_MOCK_DRIVER_HPP__

#include <MercuryCAN/driver/driverInterface.hpp>

#include <chrono>
#include <vector>

namespace Can::Driver
{
    
/**
 * \brief   Test double for the MercuryCAN driver interface.
 *
 * Provides a controllable driver for tests without CAN hardware. Frame
 * activity is exposed through txTrace and rxTrace, and the tick count can be
 * advanced explicitly with incrementTickCountMs().
 */
class MockDriver : public DriverInterface
{
public:
    friend DriverInterface;
    
    /**
     * \brief   Snapshot of a CAN frame observed by the mock driver.
     */
    struct FrameEvent
    {
        uint16_t identifier;
        std::vector<uint8_t> payload;
        std::chrono::high_resolution_clock::time_point timestamp;
    };

    /** 
     * \brief   Captured frame events for calls to transmit(), in call order. 
     */
    std::vector<FrameEvent> txTrace;

    /** 
     * \brief   Captured frame events for calls to receive(), in call order. 
     */
    std::vector<FrameEvent> rxTrace;

    /**
     * \brief   Records a transmitted frame for inspection by a test.
     *
     * \param   identifier    CAN identifier of the frame.
     * \param   data          Payload bytes to transmit.
     * \param   payloadLength Number of payload bytes.
     */
    virtual void transmit(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    /**
     * \brief   Handles a received frame and records its event for inspection.
     *
     * \param   identifier    CAN identifier of the frame.
     * \param   data          Received payload bytes.
     * \param   payloadLength Number of payload bytes.
     */
    virtual void receive(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& payloadLength
    ) override final;

    /**
     * \brief   Registers a CAN message for reception in the mock.
     *
     * \param   identifier  CAN identifier to register.
     * \param   length      Expected payload length in bytes.
     */
    virtual void addRxMessage(
        const uint16_t& identifier, 
        const uint8_t& length
    ) override final;

    /** 
     * \brief   Implements the driver's idle-sleep operation for tests. 
     */
    virtual void enterIdleSleep(void) const override final;

    /**
     * \brief   Removes a CAN identifier from the mock's receive configuration.
     *
     * \param   identifier CAN identifier to remove.
     */
    virtual void removeRxMessage(const uint16_t& identifier) override final;
    
    /** 
     * \brief   Implements activation of the transmit timer for tests. 
     */
    virtual void activateTxTimer(void) override final;

    /**
     * \brief   Sets the receiver to which incoming frames are forwarded.
     *
     * \param   recv Receiver instance; the pointer is not owned by the driver.
     */
    virtual void setReceiverInstance(Controller::Receiver* recv) override final;

    /**
     * \brief   Returns the mock driver's current millisecond tick count.
     *
     * \return  Current tick count in milliseconds.
     */
    virtual uint32_t getTickCountMs(void) const override final;

    /** 
     * \brief   Advances the mock driver's millisecond tick count by one. 
     */
    virtual void incrementTickCountMs(void) override final;

private:
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

#endif //__MOCK_MOCK_DRIVER_HPP__