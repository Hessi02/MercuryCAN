#ifndef __CAN_MODEL_MESSAGE_HPP__
#define __CAN_MODEL_MESSAGE_HPP__

#include "signal.hpp"
#include "../generic/list/container.hpp"

namespace Can::Model
{

template<typename... SignalDataTypes>
concept FitsIntoCanMessage = ((sizeof(SignalDataTypes) + ...) <= 8);

class Message
{
public:
    template<typename... SignalDataTypes>
    requires (AllowedSignalDataType<SignalDataTypes> && ...)
          && FitsIntoCanMessage<SignalDataTypes...>
    Message(
        const unsigned short identifier,
        SignalDataTypes*... signalPtrPack)
        : _payloadSize((sizeof(SignalDataTypes) + ...)),
          _identifier(identifier)
    {
        (_signals.append(
            static_cast<AnySignal_t>(
                Signal<SignalDataTypes>(signalPtrPack))),
        ...);
    }

    unsigned short getIdentifier(void) const {
        return _identifier;
    }

    std::size_t getPayloadSize(void) const {
        return _payloadSize;
    }

    unsigned char* getPayloadData(void) {
        const unsigned char signalCount = _signals.length();

        unsigned char retWriteIndex = 0;
        unsigned char* retPtr = new unsigned char[getPayloadSize()];

        for (unsigned char i = 0; i < signalCount; i++) {
            const AnySignal_t& signal = _signals.at(i);

            const std::size_t signalSize = std::visit([](auto const& s) {
                return s.getDataSize();
            }, signal);
            
            void* startPtr = std::visit([](auto& s) {
                return (void*)s.getDataPtr();
            }, signal);
            
            for (unsigned char i = 0; i < signalSize; i++)
                retPtr[retWriteIndex++] = 
                    ((unsigned char*)startPtr)[signalSize - 1 - i];
        }
        
        return retPtr;
    }

    void setPayloadData(
        const unsigned char* data, 
        const std::size_t& dataLength) 
    {
        const unsigned char signalCount = _signals.length();

        std::size_t readIndex = 0;

        for (unsigned char i = 0; i < signalCount; i++) {
            AnySignal_t& signal = _signals.at(i);

            const std::size_t signalSize = std::visit([](auto const& s) {
                return s.getDataSize();
            }, signal);

            if (readIndex + signalSize > dataLength)
                return;

            void* startPtr = std::visit([](auto& s) {
                return (void*)s.getDataPtr();
            }, signal);

            unsigned char* dst = reinterpret_cast<unsigned char*>(startPtr);

            for (std::size_t b = 0; b < signalSize; b++) 
                dst[signalSize - 1 - b] = data[readIndex++];
        }
    }

private:
    const unsigned char _payloadSize; 
    const unsigned short _identifier;    
    Generic::Container<AnySignal_t> _signals;
};

}

#endif //__CAN_MODEL_MESSAGE_HPP__
