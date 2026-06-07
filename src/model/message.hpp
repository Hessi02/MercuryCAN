#ifndef __CAN_MODEL_MESSAGE_HPP__
#define __CAN_MODEL_MESSAGE_HPP__

#include "list/container.hpp"
#include "model/signal.hpp"

namespace Can::Model {

template<typename... SignalDataTypes>
concept FitsIntoCanMessage = ((sizeof(SignalDataTypes) + ...) <= 8);

class Message {
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

        for (unsigned char i = 0; i < _signalCount; i++) {
            const AnySignal_t& signal = _signals.at(i);

            const std::size_t signalSize = std::visit(
                [](auto const& s) { return s.getDataSize(); }, signal
            );

            void* startPtr = std::visit(
                [](auto& s) { return (void*)s.getDataPtr(); }, signal
            );

            for (unsigned char i = 0; i < signalSize; i++)
                _payloadBuffer[retWriteIndex++] =
                    ((unsigned char*)startPtr)[signalSize - 1 - i];
        }

        return _payloadBuffer;
    }

    void setPayloadData(
        const unsigned char* data, const std::size_t& dataLength
    ) {
        std::size_t readIndex = 0;

        for (unsigned char i = 0; i < _signalCount; i++) {
            AnySignal_t& signal = _signals.at(i);

            const std::size_t signalSize = std::visit(
                [](auto const& s) { return s.getDataSize(); }, signal
            );

            if (readIndex + signalSize > dataLength)
                return;

            void* startPtr = std::visit(
                [](auto& s) { return (void*)s.getDataPtr(); }, signal
            );

            unsigned char* dst = reinterpret_cast<unsigned char*>(startPtr);

            for (std::size_t b = 0; b < signalSize; b++)
                dst[signalSize - 1 - b] = data[readIndex++];
        }
    }

    void setUpdateFlag(const bool& flag) {
        _updateFlag = flag;
    }

    bool getUpdateFlag(void) const {
        return _updateFlag;
    }

private:
    const unsigned char _payloadSize;
    const unsigned short _identifier;
    unsigned char _signalCount;
    bool _updateFlag = false;
    Generic::Container<AnySignal_t> _signals;
    mutable unsigned char _payloadBuffer[8] = {};
};

}

#endif  //__CAN_MODEL_MESSAGE_HPP__
