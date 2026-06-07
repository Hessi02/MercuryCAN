#ifndef __CAN_CONTROLLER_TRANSMITTER_HPP__
#define __CAN_CONTROLLER_TRANSMITTER_HPP__

#include "generic/list/container.hpp"
#include "model/cyclicMessage.hpp"

namespace Can::Controller {

class Transmitter {
public:
    void sendMessage(Model::Message& message) const;
    void addCyclicMessage(Model::CyclicMessage& message);
    unsigned char getMessageCount(void) const;

    static void processTransmitCycle(void);

private:
    static inline unsigned long _tickCountMs = 0;
    static inline unsigned char _messageCount = 0;
    static inline Generic::Container<Model::CyclicMessage> _cyclicMessages;
};

}

#endif  //__CAN_CONTROLLER_TRANSMITTER_HPP__
