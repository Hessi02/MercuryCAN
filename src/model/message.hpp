#ifndef __CAN_MODEL_MESSAGE_HPP__
#define __CAN_MODEL_MESSAGE_HPP__

#include "list/container.hpp"
#include "bareSignal/metaObject.hpp"
#include "model/signal.hpp"

namespace Can::Model {

template<typename... SignalDataTypes>
concept FitsIntoCanMessage = ((sizeof(SignalDataTypes) + ...) <= 8);

class Message : public BareSignal::MetaObject
{
public:
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes> && ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    Message(const unsigned short identifier, SignalDataTypes*... signalPtrPack)
        : _payloadSize((sizeof(SignalDataTypes) + ...)),
          _identifier(identifier) {
        (_signals.append(
             static_cast<AnySignal_t>(Signal<SignalDataTypes>(signalPtrPack))
         ),
         ...);

        _signalCount = _signals.length();
    }

    unsigned short getIdentifier(void) const {
        return _identifier;
    }

    std::size_t getPayloadSize(void) const {
        return _payloadSize;
    }

    unsigned char* getPayloadData(void) {
        unsigned char retWriteIndex = 0;

        for (unsigned char index = 0; index < _signalCount; index++)
            appendSignalPayload(_signals.at(index), retWriteIndex);

        return _payloadBuffer;
    }

    void setPayloadData(
        const unsigned char* data, const std::size_t& dataLength
    ) {
        std::size_t readIndex = 0;

        for (unsigned char index = 0; index < _signalCount; index++) {
            if (!applySignalPayload(
                    _signals.at(index), data, dataLength, readIndex
                ))
                return;
        }
    }

    void setUpdateFlag(const bool& flag) {
        _updateFlag = flag;
    }

    bool getUpdateFlag(void) const {
        return _updateFlag;
    }

signals:
    void preSend(unsigned long tickCountMs) {
        executeAllCallbacks(this, &Message::preSend, tickCountMs);
    }    

    void sent(unsigned long tickCountMs) {
        executeAllCallbacks(this, &Message::sent, tickCountMs);
    }

    void received(unsigned long tickCountMs) {
        executeAllCallbacks(this, &Message::received, tickCountMs);
    }

private:
    void appendSignalPayload(
        const AnySignal_t& signal, unsigned char& writeIndex
    ) const {
        const std::size_t signalSize = std::visit(
            [](auto const& value) { return value.getDataSize(); }, signal
        );
        const volatile unsigned char* startPtr = std::visit(
            [](auto const& value) -> const volatile unsigned char* {
                return reinterpret_cast<const volatile unsigned char*>(
                    value.getDataPtr()
                );
            }, signal
        );

        for (std::size_t index = 0; index < signalSize; index++)
            _payloadBuffer[writeIndex++] = startPtr[signalSize - 1 - index];
    }

    bool applySignalPayload(
        AnySignal_t& signal,
        const unsigned char* data,
        const std::size_t& dataLength,
        std::size_t& readIndex
    ) {
        const std::size_t signalSize = std::visit(
            [](auto const& value) { return value.getDataSize(); }, signal
        );
        if (readIndex + signalSize > dataLength)
            return false;

        volatile unsigned char* startPtr = std::visit(
            [](auto& value) -> volatile unsigned char* {
                return reinterpret_cast<volatile unsigned char*>(
                    value.getDataPtr()
                );
            }, signal
        );
        for (std::size_t index = 0; index < signalSize; index++)
            startPtr[signalSize - 1 - index] = data[readIndex++];
        return true;
    }

    const unsigned char _payloadSize;
    const unsigned short _identifier;
    unsigned char _signalCount;
    bool _updateFlag = false;
    Generic::Container<AnySignal_t> _signals;
    mutable unsigned char _payloadBuffer[8] = {};
};

}

#endif  //__CAN_MODEL_MESSAGE_HPP__
