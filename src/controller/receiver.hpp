#ifndef __CAN_CONTROLLER_RECEIVER_HPP__
#define __CAN_CONTROLLER_RECEIVER_HPP__

#include "model/cyclicMessage.hpp"

namespace Can::Controller {

class Receiver {
public:
    void awaitMessage(Model::Message& message);

    void addCyclicMessage(Model::CyclicMessage& message);

    void processRxData(
        const unsigned short& identifier,
        const unsigned char* data,
        const std::size_t& dataLength
    );

private:
    unsigned char _messageCount = 0;
    Generic::Container<Model::CyclicMessage> _cyclicMessages;
};

}

#endif  //__CAN_CONTROLLER_RECEIVER_HPP__