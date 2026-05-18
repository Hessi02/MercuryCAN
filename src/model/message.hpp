#ifndef __CAN_MODEL_MESSAGE_HPP__
#define __CAN_MODEL_MESSAGE_HPP__

#include <memory>

#include "signal.hpp"
#include "../generic/list/container.hpp"

namespace Can::Model
{

/**
 *  \concept    FitsIntoCanMessage
 *  \brief      Restricts signals to fit into a single CAN frame.
 *  \tparam     SignalDataTypes passes a variadic list of all signal types.
 * 
 *  This concept is used to ensure that the total size of all signal types does
 *  not exceed the standard CAN frame's payload size of eight bytes. The check
 *  is performed at compile-time. 
 */
template<typename... SignalDataTypes>
concept FitsIntoCanMessage =
    ((sizeof(SignalDataTypes) + ...) <= 8);

/**
 *  \class      Message
 *  \brief      A message refers to the combination of one or more signals.
 *  \tparam     SignalDataType specifies the signals' types to use.
 * 
 *  The 'Message' class represents a standard CAN message / frame and combines
 *  one or more signals to a contiguous bit sequence which gets physically
 *  written to the bus. The class thus represents not only an abstraction and 
 *  combination of signals, but also an interface from the abstract data 
 *  structure to the low-level bit structure that is written to the bus.
 */
class Message
{
public:
    /**
     *  \brief          Initializes instance by creating signals from pointers.
     *  \tparam         SignalDataTypes specifies signals' types variadically.
     *  \param[in]      identifier passes the CAN message's identifier. 
     *  \param[in|out]  signalPtrPack passes signals' data ptr as param pack.
     */
    template<typename... SignalDataTypes>
    requires (AllowedSignalDataType<SignalDataTypes> && ...)
          && FitsIntoCanMessage<SignalDataTypes...>
    Message(
        const unsigned short identifier,
        SignalDataTypes*... signalPtrPack)
        : _payloadSize((sizeof(SignalDataTypes) + ...)),
          _identifier(identifier)
    {
        (_signals.append(
            static_cast<AnySignal_t>(
                Signal<SignalDataTypes>(signalPtrPack))),
        ...);
    }

    /**
     *  \brief      Getter method for the CAN message's identifier.
     *  \return     Message ID represented as unsigned short.
     */
    unsigned short getIdentifier(void) const {
        return _identifier;
    }

    /**
     *  \brief      Getter method for the requiered payload size.
     *  \return     The payload size represented as std::size_t. 
     */
    std::size_t getPayloadSize(void) const {
        return _payloadSize;
    }

    /**
     *  \brief      Converts the signals' data to simple byte array.
     *  \return     A unqiue pointer to an allocated array of unsigned chars.
     */
    std::unique_ptr<unsigned char[]> getPayloadData(void) {
        unsigned char retWriteIndex = 0;
        auto retPtr = std::make_unique<unsigned char[]>(getPayloadSize());

        for (const AnySignal_t& signal : _signals) {
            const std::size_t signalSize = std::visit(
                [](auto const& s) { return s.getDataSize(); }, signal);

            void* startPtr = std::visit(
                [](auto& s) { return (void*)s.getDataPtr(); }, signal);

            for (unsigned char i = 0; i < signalSize; i++)
                retPtr[retWriteIndex++] =
                    ((unsigned char*)startPtr)[signalSize - 1 - i];
        }

        return retPtr;
    }

private:
    const unsigned char _payloadSize; 
    const unsigned short _identifier;    
    Generic::Container<AnySignal_t> _signals;
};

}

#endif //__CAN_MODEL_MESSAGE_HPP__
