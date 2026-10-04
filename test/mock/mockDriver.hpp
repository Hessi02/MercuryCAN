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
 * Provides a controllable driver for tests without CAN hardware. Frame activity 
 * is exposed through txTrace and rxTrace, and the tick count can be advanced 
 * explicitly with incrementTickCountMs().
 */
class MockDriver : public DriverInterface
{
public:
    friend DriverInterface;
    
    /**
     * \brief   Snapshot of a CAN frame observed by the mock driver.
     * 
     * The FrameEvent structure captures the essential details of a CAN frame,
     * including its identifier, payload, and the timestamp of when it was
     * transmitted or received. This information is useful for verifying the
     * behavior of the driver during testing, allowing tests to assert that
     * frames are sent and received as expected.
     */
    struct FrameEvent
    {
        uint16_t identifier;
        std::vector<uint8_t> payload;
        std::chrono::high_resolution_clock::time_point timestamp;
    };

    /** 
     * \brief   Captured frame events for calls to transmit(), in call order. 
     * 
     * The txTrace vector stores a sequence of FrameEvent instances representing
     * the frames that have been transmitted by the mock driver. Each event
     * includes the identifier, payload, and timestamp of the transmitted frame,
     * allowing tests to verify that the correct frames were sent and in the
     * expected order. This trace can be used to assert the behavior of the
     * driver during testing, ensuring that the library's transmission logic
     * functions correctly.
     */
    std::vector<FrameEvent> txTrace;

    /** 
     * \brief   Captured frame events for calls to receive(), in call order. 
     * 
     * The rxTrace vector stores a sequence of FrameEvent instances representing
     * the frames that have been received by the mock driver. Each event 
     * includes the identifier, payload, and timestamp of the received frame,
     * allowing tests to verify that the correct frames were received and in the
     * expected order. This trace can be used to assert the behavior of the
     * driver during testing, ensuring that the library's reception logic
     * functions correctly.
     */
    std::vector<FrameEvent> rxTrace;

    /**
     * \brief   Records a transmitted frame for inspection by a test.
     * 
     * This method captures the details of a transmitted CAN frame that gets 
     * called by Transmitter's code under test. It records the frame's 
     * identifier, payload, and the current timestamp, storing this information 
     * in the txTrace vector. Tests can use this method to verify that the 
     * correct frames are being transmitted by the driver, ensuring that the 
     * library's transmission logic behaves as expected.
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
     * This method simulates the reception of a CAN frame by the mock driver. It
     * records the frame's identifier, payload, and the current timestamp, 
     * storing this information in the rxTrace vector. Tests can use this method 
     * to verify that the correct frames are being received by the driver, 
     * ensuring that the library's reception logic behaves as expected. The 
     * method also forwards the received frame to the registered Receiver
     * instance, if one is registered.
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
     * This method allows tests to configure the mock driver to expect incoming
     * frames with a specific CAN identifier and payload length. It simulates
     * the behavior of a real driver by storing the registration information,
     * allowing the mock to verify that received frames match the expected
     * configuration. Tests can use this method to ensure that the driver is
     * correctly configured to receive the desired messages, and that the
     * library's reception logic behaves as expected when frames are received.
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
     * 
     * This method simulates the behavior of a real driver entering an idle 
     * sleep mode. In the context of testing, it allows tests to verify that the 
     * driver correctly transitions to a low-power state when requested. The 
     * method does not perform any actual sleep operation, but it can be used to 
     * assert that the library's logic for entering idle sleep behaves as 
     * expected.
     */
    virtual void enterIdleSleep(void) const override final;

    /**
     * \brief   Removes a CAN identifier from the mock's receive configuration.
     * 
     * This method allows tests to simulate the removal of a CAN identifier from 
     * the mock driver's receive configuration. It updates the internal state of 
     * the mock driver to reflect that the specified identifier is no longer 
     * being monitored for incoming frames. Tests can use this method to verify 
     * that the driver correctly handles the removal of receive configurations 
     * and that the library's reception logic behaves as expected when frames 
     * for the removed identifier are received.
     *
     * \param   identifier CAN identifier to remove.
     */
    virtual void removeRxMessage(const uint16_t& identifier) override final;
    
    /** 
     * \brief   Implements activation of the transmit timer for tests. 
     * 
     * This method simulates the activation of a timer used for cyclic 
     * transmissions in the mock driver. In the context of testing, it allows 
     * tests to verify that the driver correctly starts the timer when 
     * requested. The method does not perform any actual timing operations, but 
     * it can be used to assert that the library's logic for activating the 
     * transmit timer behaves as expected.
     */
    virtual void activateTxTimer(void) override final;

    /**
     * \brief   Sets the receiver to which incoming frames are forwarded.
     * 
     * This method allows tests to register a Receiver instance with the mock
     * driver. When a frame is received, the mock driver will forward the frame
     * data to the registered Receiver for processing. The method does not take
     * ownership of the Receiver instance; it is the responsibility of the test
     * code to ensure that the Receiver remains valid for the duration of its
     * use by the mock driver. Tests can use this method to verify that the
     * driver correctly forwards received frames to the appropriate Receiver,
     * ensuring that the library's reception logic behaves as expected.
     *
     * \param   recv Receiver instance; the pointer is not owned by the driver.
     */
    virtual void setReceiverInstance(Controller::Receiver* recv) override final;

    /**
     * \brief   Returns the mock driver's current millisecond tick count.
     * 
     * This method provides the current tick count in milliseconds, which can be
     * used by tests to verify the timing behavior of the driver. The tick count
     * is advanced explicitly by calling incrementTickCountMs(), allowing tests 
     * to simulate the passage of time and verify that the library's timing 
     * logic behaves as expected. The method returns the current value of the
     * _tickCountMs member variable, which is updated by the mock driver during
     * testing.
     *
     * \return  Current tick count in milliseconds.
     */
    virtual uint32_t getTickCountMs(void) const override final;

    /** 
     * \brief   Advances the mock driver's millisecond tick count by one. 
     * 
     * This method simulates the passage of time in the mock driver by 
     * incrementing the _tickCountMs member variable by one millisecond. Tests 
     * can use this method to control the timing behavior of the driver and 
     * verify that the library's timing logic behaves as expected. The method 
     * does not perform any actual timing operations, but it allows tests to 
     * simulate the passage of time and verify that the driver correctly handles 
     * timing-related events, such as cyclic transmissions and timeouts.
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