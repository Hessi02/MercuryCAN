/**
 * \file    message.hpp
 * \brief   Defines the CAN message model and its payload handling.
 *
 * A Message associates a CAN identifier with non-owning pointers to signal
 * data. It serializes signal values in construction order, reversing the byte
 * order within each value, into a payload of at most eight bytes. Through its
 * MetaObject base, it dispatches registered callbacks before sending, after
 * sending, and after received payload data has been applied. 
 * 
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 * 
 * SPDX-License-Identifier: Apache-2.0 
 */

#ifndef __MERCURYCAN_MODEL_MESSAGE_HPP__
#define __MERCURYCAN_MODEL_MESSAGE_HPP__

#include "support/list/container.hpp"
#include "support/bareSignal/metaObject.hpp"
#include "model/signal.hpp"

namespace Can::Model {

/**
 * \brief   Checks whether the given signal data types fit into a CAN payload.
 *
 * The sum of the sizes of all types must not exceed the eight-byte payload
 * limit used by this message model. This is determined by the maximum data 
 * length of a classic CAN bus. Even though extensions such as CAN FD or CAN XL 
 * allow for more bytes, these methods are not available for the AVR.
 *
 * \tparam  SignalDataTypes passes the signal data types to check.
 */
template<typename... SignalDataTypes>
concept FitsIntoCanMessage = ((sizeof(SignalDataTypes) + ...) <= 8);

/**
 * \brief   Represents a CAN message and the signals contained in its payload.
 *
 * A Message stores an identifier and non-owning references to its signal data.
 * The referenced objects must remain valid for the lifetime of the Message.
 * Payload data is serialized one signal at a time, with each signal's bytes
 * written in reverse order. The total payload size cannot exceed eight bytes as
 * classic CAN is used for transmission.
 *
 * \see     FitsIntoCanMessage
 * \see     Signal
 */
class Message : public BareSignal::MetaObject
{
public:
    /**
     * \brief   Constructs a message from its identifier and signal references.
     *
     * Each provided signal type must be allowed by AllowedSignalDataType, and 
     * the sum of their sizes must fit into the message's eight-byte payload.
     *
     * \tparam  SignalDataTypes passes the types of the signal data.
     * \param   identifier passes the CAN identifier for this message.
     * \param   signalPtrPack passes pointers to the signal data objects.
     *
     * \see     AllowedSignalDataType
     * \see     FitsIntoCanMessage
     */
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    Message(const uint16_t identifier, SignalDataTypes*... signalPtrPack)
        : _payloadSize((sizeof(SignalDataTypes) + ...)),
          _identifier(identifier) {
        (_signals.append(
             static_cast<AnySignal_t>(Signal<SignalDataTypes>(signalPtrPack))
         ),
         ...);

        _signalCount = _signals.length();
    }

    /**
     * \brief   Returns the CAN identifier of this message.
     *
     * \return  CAN identifier supplied to the constructor.
     */
    uint16_t getIdentifier(void) const {
        return _identifier;
    }

    /**
     * \brief   Returns the combined size of the message's signal data.
     *
     * \return  Payload size in bytes.
     */
    std::size_t getPayloadSize(void) const {
        return _payloadSize;
    }

    /**
     * \brief   Serializes the current signal data into the internal payload.
     *
     * Each signal's bytes are appended in reverse order. The returned pointer
     * refers to storage owned by this Message and is overwritten by the next
     * call to this method.
     *
     * \return  Pointer to the serialized payload buffer.
     */
    unsigned char* getPayloadData(void) {
        uint8_t retWriteIndex = 0;

        for (uint8_t index = 0; index < _signalCount; index++)
            appendSignalPayload(_signals.at(index), retWriteIndex);

        return _payloadBuffer;
    }

    /**
     * \brief   Applies payload bytes to the message's signal data.
     *
     * Bytes are consumed in signal order, and each signal's bytes are read in
     * reverse order. If the input is too short for a signal, processing stops;
     * any earlier signals may already have been updated.
     *
     * \param   data passes the payload bytes to apply.
     * \param   dataLength passes the number of available payload bytes.
     */
    void setPayloadData(
        const uint8_t* data, const std::size_t& dataLength
    ) {
        std::size_t readIndex = 0;

        for (uint8_t index = 0; index < _signalCount; index++) {
            if (!applySignalPayload(
                    _signals.at(index), data, dataLength, readIndex
                ))
                return;
        }
    }

    /**
     * \brief   Sets the message update flag.
     *
     * The flag is not modified automatically by setPayloadData.
     *
     * \param   flag passes the new flag value.
     */
    void setUpdateFlag(const bool& flag) {
        _updateFlag = flag;
    }

    /**
     * \brief   Returns the current message update flag.
     *
     * \return  True if the flag is set; otherwise false.
     */
    bool getUpdateFlag(void) const {
        return _updateFlag;
    }

signals:
    /**
     * \brief   Notifies observers immediately before a message is sent.
     *
     * \param   tickCountMs passes the elapsed tick count in milliseconds.
     */
    void preSend(uint32_t tickCountMs) {
        executeAllCallbacks(this, &Message::preSend, tickCountMs);
    }    

    /**
     * \brief   Notifies observers after a message has been sent.
     *
     * \param   tickCountMs passes the elapsed tick count in milliseconds.
     */
    void sent(uint32_t tickCountMs) {
        executeAllCallbacks(this, &Message::sent, tickCountMs);
    }

    /**
     * \brief   Notifies observers after received payload data is applied.
     *
     * \param   tickCountMs passes the elapsed tick count in milliseconds.
     */
    void received(uint32_t tickCountMs) {
        executeAllCallbacks(this, &Message::received, tickCountMs);
    }

private:
    /**
     * \brief   Appends one signal's bytes to the payload buffer.
     *
     * Signal bytes are copied to consecutive payload positions, starting at
     * writeIndex, while source bytes are read from last to first. Afterwards,
     * writeIndex points to the next free position, advanced by the signal size.
     *
     * \param   signal passes the signal whose data is serialized.
     * \param   writeIndex passes the payload write index and updates the next.
     */
    void appendSignalPayload(
        const AnySignal_t& signal, uint8_t& writeIndex
    ) const {
        const std::size_t signalSize = std::visit(
            [](auto const& value) { return value.getDataSize(); }, signal
        );

        const volatile uint8_t* startPtr = std::visit(
            [](auto const& value) -> const volatile uint8_t* {
                return reinterpret_cast<const volatile uint8_t*>(
                    value.getDataPtr()
                );
            }, signal
        );

        for (std::size_t index = 0; index < signalSize; index++)
            _payloadBuffer[writeIndex++] = startPtr[signalSize - 1 - index];
    }

    /**
     * \brief   Applies bytes from the input payload to one signal.
     *
     * If the remaining input is shorter than the signal data, no bytes are
     * applied to this signal and false is returned. Otherwise bytes are read
     * in reverse order and readIndex is advanced.
     *
     * \param   signal passes the signal whose data is updated.
     * \param   data passes the input payload bytes.
     * \param   dataLength passes the total number of available input bytes.
     * \param   readIndex passes the input index and receives the next.
     *
     * \return  True if the signal data was fully applied; otherwise false.
     */
    bool applySignalPayload(
        AnySignal_t& signal,
        const uint8_t* data,
        const std::size_t& dataLength,
        std::size_t& readIndex
    ) {
        const std::size_t signalSize = std::visit(
            [](auto const& value) { return value.getDataSize(); }, signal
        );
        
        if (readIndex + signalSize > dataLength)
            return false;

        volatile uint8_t* startPtr = std::visit(
            [](auto& value) -> volatile uint8_t* {
                return reinterpret_cast<volatile uint8_t*>(
                    value.getDataPtr()
                );
            }, signal
        );

        for (std::size_t index = 0; index < signalSize; index++)
            startPtr[signalSize - 1 - index] = data[readIndex++];
            
        return true;
    }

    /**
     * \brief   Total size of the message's signal data in bytes.
     */
    const uint8_t _payloadSize;

    /**
     * \brief   CAN identifier associated with this message.
     */
    const uint16_t _identifier;

    /**
     * \brief   Number of signal references stored in _signals.
     */
    uint8_t _signalCount;

    /**
     * \brief   Explicitly managed flag indicating a message update.
     */
    bool _updateFlag = false;

    /**
     * \brief   Non-owning references to the signals contained in this message.
     */
    Generic::Container<AnySignal_t> _signals;

    /**
     * \brief   Internal buffer used to serialize the message payload.
     *
     * The buffer has the maximum supported CAN payload capacity of eight bytes.
     */
    mutable uint8_t _payloadBuffer[8] = {};
};

}

#endif  // __MERCURYCAN_MODEL_MESSAGE_HPP__
