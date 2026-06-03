#ifndef __CAN_MODEL_CYCLIC_MESSAGE_HPP__
#define __CAN_MODEL_CYCLIC_MESSAGE_HPP__

#include "message.hpp"

namespace Can::Model
{

class CyclicMessage : public Message
{
public:
    template<typename... SignalDataTypes>
    requires (AllowedSignalDataType<SignalDataTypes>, ...)
          && FitsIntoCanMessage<SignalDataTypes...>
    CyclicMessage(
        const unsigned short& identifier,
        const unsigned short& cycleTime,
        SignalDataTypes* ... signalReferences) 
        : Message(identifier, signalReferences...), 
          _cycleTime(cycleTime)              
    {}

    unsigned short getCycleTime(void) const {
        return _cycleTime; 
    }

private:
    const unsigned short _cycleTime;
};

}

#endif //__CAN_MODEL_CYCLIC_MESSAGE_HPP__