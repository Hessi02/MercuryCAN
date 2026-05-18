#ifndef __CAN_MODEL_CYCLIC_MESSAGE_HPP__
#define __CAN_MODEL_CYCLIC_MESSAGE_HPP__

#include "message.hpp"

namespace Can::Model
{

/**
 *  \class      CyclicMessage
 *  \brief      A message that is written to the bus cyclically.
 * 
 *  While the base class only represents a standard CAN frame, this class adds
 *  the timing information for fixed transmitting intervals. Note that this 
 *  class only contains the timing informations while controller and driver 
 *  classes are responsible for execution of the cyclic write operation.
 */
class CyclicMessage : public Message
{
public:
    /**
     *  \brief          Initializes base and instance with added cycle time.
     *  \tparam         SignalDataTypes specifies signals' types variadically.
     *  \param[in]      identifier passes the CAN message's identifier. 
     *  \param[in]      cycleTime passes the time between CAN messages on bus.
     *  \param[in|out]  signalPtrPack passes signals' data ptr as param pack.
     */
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

    /**
     *  \brief      Getter method for the instance's cycle time.
     *  \return     Cycle time represented as a unsiged short.
     */
    unsigned short getCycleTime(void) const {
        return _cycleTime; 
    }

private:
    const unsigned short _cycleTime;
};

}

#endif //__CAN_MODEL_CYCLIC_MESSAGE_HPP__