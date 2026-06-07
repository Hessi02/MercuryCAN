#ifndef __CAN_CONTROLLER_DRIVER_HPP__
#define __CAN_CONTROLLER_DRIVER_HPP__

#include <cstdlib>

namespace Can::Controller {

class Receiver;

class Driver {
public:
    static Driver& getInstance(void);

    void transmit(
        const unsigned short& identifier,
        const unsigned char* data,
        const std::size_t& payloadLength
    );

    void receive(
        const unsigned short& identifier,
        const unsigned char* data,
        const std::size_t& payloadLength
    );

    void addRxMessage(
        const unsigned short& identifier, const unsigned char& length
    );

    void removeRxMessage(const unsigned short& identifier);

    void setReceiverInstance(Receiver* recv);
    void activateTxTimer(void);

private:
    void initHardware(void) const;
    unsigned char reserveMessageObject(void);
    void freeMessageObject(const unsigned char& index);
    void resetMessageObjects(void) const;

    Driver(void);

    Receiver* _receiver = nullptr;

    static inline constexpr unsigned char _messageObjectCount = 15;
    static inline unsigned short _usedMessageObjectMask = 0;
};

}

#endif  //__CAN_CONTROLLER_DRIVER_HPP__
