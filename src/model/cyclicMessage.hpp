#ifndef __CAN_MODEL_CYCLIC_MESSAGE_HPP__
#define __CAN_MODEL_CYCLIC_MESSAGE_HPP__

#include "model/message.hpp"

namespace Can::Model {

class CyclicMessage : public Message {
public:
    template<typename... SignalDataTypes>
        requires(AllowedSignalDataType<SignalDataTypes>, ...) &&
                    FitsIntoCanMessage<SignalDataTypes...>
    CyclicMessage(
        const uint16_t& identifier,
        const uint16_t& cycleTime,
        SignalDataTypes*... signalReferences
    )
        : Message(identifier, signalReferences...), _cycleTime(cycleTime) {
    }

    uint16_t getCycleTime(void) const {
        return _cycleTime;
    }

private:
    const uint16_t _cycleTime;
};

}

#endif  //__CAN_MODEL_CYCLIC_MESSAGE_HPP__