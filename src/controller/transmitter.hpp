#ifndef __CAN_CONTROLLER_TRANSMITTER_HPP__
#define __CAN_CONTROLLER_TRANSMITTER_HPP__

#include "list/container.hpp"
#include "model/cyclicMessage.hpp"

namespace Can::Controller {

class Transmitter {
public:
    Transmitter(void);

    void sendMessage(Model::Message& message) const;
    void addCyclicMessage(Model::CyclicMessage& message);
    unsigned char getMessageCount(void) const;
    static void processTransmitCycle(void);

private:
    static void processMessage(
        const unsigned char& index,
        const unsigned long& tickCountMs,
        bool* dueMessages
    );

    static void transmitMessage(
        const unsigned char& index, const bool* dueMessages
    );

    static void emitSentMessage(
        const unsigned char& index,
        const unsigned long& tickCountMs,
        const bool* dueMessages
    );
    
    static inline unsigned char _messageCount = 0;
    static inline Generic::Container<Model::CyclicMessage*> _cyclicMessages;
};

}

#endif  //__CAN_CONTROLLER_TRANSMITTER_HPP__
