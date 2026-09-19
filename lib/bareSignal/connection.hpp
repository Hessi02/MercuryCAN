#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__

#include <type_traits>

template<class Base, class Derived>
concept isDerived = std::is_base_of<Base, Derived>::value;

namespace BareSignal 
{

class MetaObject;

template<class... ParamPack>
class Connection
{
public:
    typedef void(MetaObject::*Signal_t)(ParamPack...);
    typedef void(MetaObject::*Callback_t)(ParamPack...);
    typedef void(*StaticCallback_t)(ParamPack...);

    template<class Sender,
             class SenderBase,
             class Receiver,
             class ReceiverBase>
    requires isDerived<SenderBase, Sender> &&
             isDerived<ReceiverBase, Receiver>
    Connection(Sender* sender,
               void(SenderBase::*signal)(ParamPack...),
               Receiver* receiver,
               void(ReceiverBase::*callback)(ParamPack...)
    ) : _sender(sender),
        _signal(static_cast<Signal_t>(signal)),
        _receiver(receiver),
        _callback(static_cast<Callback_t>(callback)),
        _staticCallback(nullptr)
    {}

    template<class Sender,
             class SenderBase>
    requires isDerived<SenderBase, Sender>
    Connection(Sender* sender,
               void(SenderBase::*signal)(ParamPack...),
               StaticCallback_t callback
    ) : _sender(sender),
        _signal(static_cast<Signal_t>(signal)),
        _receiver(nullptr),
        _callback(nullptr),
        _staticCallback(callback)
    {}

    void executeCallback(ParamPack... args) {
        if (_receiver && _callback)
            (*_receiver.*_callback)(args...);
        else if (_staticCallback)
            (*_staticCallback)(args...);
    }

    bool isSender(const MetaObject* sender) const {
        return sender == _sender;
    }

    bool isReceiver(const MetaObject* receiver) const {
        return receiver == _receiver;
    }

    bool isSignal(const Signal_t& signal) const {
        return signal == _signal;
    }

    bool isCallback(const Callback_t& callback) const {
        return callback == _callback;
    }

    bool isCallback(const StaticCallback_t& callback) const {
        return callback == _staticCallback;
    }

private:
    MetaObject* _sender;
    MetaObject* _receiver;

    const Signal_t _signal;
    const Callback_t _callback;
    const StaticCallback_t _staticCallback;
};

}

#endif //__CONNECTION_HPP__