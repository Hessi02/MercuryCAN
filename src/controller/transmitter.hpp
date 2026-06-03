#ifndef __CAN_CONTROLLER_TRANSMITTER_HPP__
#define __CAN_CONTROLLER_TRANSMITTER_HPP__

#include "../model/cyclicMessage.hpp"
#include "../generic/list/container.hpp"

namespace Can::Controller
{

class Transmitter
{
public:
    Transmitter(void); 

    void addMessage(Model::Message& message) const; 
    void addCyclicMessage(Model::CyclicMessage& message);
    unsigned char getMessageCount(void) const;
    
    static inline void processTransmitCycle(void);
    static inline void incrementTickCount(void);
    
private:
    static void activateTimer(void); 
    
    static inline unsigned long tickCountMs = 0;
    static inline Generic::Container<Model::CyclicMessage> _cyclicMessages;
};

}

#endif //__CAN_CONTROLLER_TRANSMITTER_HPP__
