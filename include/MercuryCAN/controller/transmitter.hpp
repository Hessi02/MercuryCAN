#ifndef __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__
#define __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__

#include <MercuryCAN/support/list/container.hpp>
#include <MercuryCAN/model/cyclicMessage.hpp>

namespace Can::Controller {

class Transmitter {
public:
    Transmitter(void);

    void sendMessage(Model::Message& message) const;

    void addCyclicMessage(Model::CyclicMessage& message);

    uint8_t getMessageCount(void) const;

    static void processTransmitCycle(void);

private:
    static void processMessage(
        const uint8_t& index,
        const uint32_t& tickCountMs,
        bool* dueMessages
    );

    static void transmitMessage(
        const uint8_t& index, const bool* dueMessages
    );

    static void emitSentMessage(
        const uint8_t& index,
        const uint32_t& tickCountMs,
        const bool* dueMessages
    );
    
    static inline uint8_t _messageCount = 0;
    static inline Generic::Container<Model::CyclicMessage*> _cyclicMessages;
};

}

#endif  // __MERCURYCAN_CONTROLLER_TRANSMITTER_HPP__
