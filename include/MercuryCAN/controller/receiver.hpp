#ifndef __MERCURYCAN_CONTROLLER_RECEIVER_HPP__
#define __MERCURYCAN_CONTROLLER_RECEIVER_HPP__

#include <MercuryCAN/model/cyclicMessage.hpp>

namespace Can::Controller {

class Receiver {
public:
    void awaitMessage(Model::Message& message);

    void addCyclicMessage(Model::CyclicMessage& message);

    void processRxData(
        const uint16_t& identifier,
        const uint8_t* data,
        const std::size_t& dataLength
    );

private:
    uint8_t _messageCount = 0;
    Generic::Container<Model::CyclicMessage> _cyclicMessages;
};

}

#endif  // __MERCURYCAN_CONTROLLER_RECEIVER_HPP__