/**
 * \file    connection.hpp
 * \brief   Defines signal-to-callback connection records.
 *
 * Connection stores the sender, signal, and either a receiver member callback
 * or a static callback, and provides operations to execute and inspect that
 * connection.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_SUPPORT_SIGNAL_SLOT_CONNECTION_HPP__
#define __MERCURYCAN_SUPPORT_SIGNAL_SLOT_CONNECTION_HPP__

#include <type_traits>

/**
 * \brief   Checks whether one type derives from another.
 *
 * \tparam  Base passes the base type.
 * \tparam  Derived passes the type to check.
 */
template<class Base, class Derived>
concept isDerived = std::is_base_of<Base, Derived>::value;

namespace SignalSlot
{

class MetaObject;

/**
 * \brief   Stores a connection between a signal and a callback.
 *
 * A connection can target either a member callback on a receiver object or a
 * static callback function. Sender and receiver pointers are non-owning.
 *
 * \tparam  ParamPack passes the signal and callback parameter types.
 */
template<class... ParamPack>
class Connection
{
public:
    /**
     * \brief   Member-function pointer type used for a signal.
     */
    typedef void(MetaObject::*Signal_t)(ParamPack...);

    /**
     * \brief   Member-function pointer type used for a receiver callback.
     */
    typedef void(MetaObject::*Callback_t)(ParamPack...);

    /**
     * \brief   Function pointer type used for a static callback.
     */
    typedef void(*StaticCallback_t)(ParamPack...);

    /**
     * \brief   Connects a sender signal to a receiver member callback.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \tparam  Receiver passes the concrete receiver type.
     * \tparam  ReceiverBase passes the base type declaring the callback.
     * \param   sender passes the sender object.
     * \param   signal passes the signal member function.
     * \param   receiver passes the receiver object.
     * \param   callback passes the receiver member function to invoke.
     */
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
        _receiver(receiver),
        _signal(static_cast<Signal_t>(signal)),
        _callback(static_cast<Callback_t>(callback)),
        _staticCallback(nullptr)
    {}

    /**
     * \brief   Connects a sender signal to a static callback function.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \param   sender passes the sender object.
     * \param   signal passes the signal member function.
     * \param   callback passes the static function to invoke.
     */
    template<class Sender,
             class SenderBase>
    requires isDerived<SenderBase, Sender>
    Connection(Sender* sender,
               void(SenderBase::*signal)(ParamPack...),
               StaticCallback_t callback
    ) : _sender(sender),
        _receiver(nullptr),
        _signal(static_cast<Signal_t>(signal)),
        _callback(nullptr),
        _staticCallback(callback)
    {}

    /**
     * \brief   Invokes the callback associated with this connection.
     *
     * A receiver member callback is invoked when both its receiver and
     * callback pointer are set; otherwise, a set static callback is invoked.
     *
     * \param   args passes the arguments forwarded to the callback.
     */
    void executeCallback(ParamPack... args) {
        if (_receiver && _callback)
            (*_receiver.*_callback)(args...);
        else if (_staticCallback)
            (*_staticCallback)(args...);
    }

    /**
     * \brief   Checks whether the supplied object is this connection's sender.
     *
     * \param   sender passes the object to compare.
     *
     * \return  True if the pointer matches the stored sender.
     */
    bool isSender(const MetaObject* sender) const {
        return sender == _sender;
    }

    /**
     * \brief   Checks whether the supplied object is this connection's receiver.
     *
     * \param   receiver passes the object to compare.
     *
     * \return  True if the pointer matches the stored receiver.
     */
    bool isReceiver(const MetaObject* receiver) const {
        return receiver == _receiver;
    }

    /**
     * \brief   Checks whether the supplied signal is this connection's signal.
     *
     * \param   signal passes the signal member function to compare.
     *
     * \return  True if the member function matches the stored signal.
     */
    bool isSignal(const Signal_t& signal) const {
        return signal == _signal;
    }

    /**
     * \brief   Checks whether the supplied member callback matches this connection.
     *
     * \param   callback passes the receiver member function to compare.
     *
     * \return  True if the member function matches the stored callback.
     */
    bool isCallback(const Callback_t& callback) const {
        return callback == _callback;
    }

    /**
     * \brief   Checks whether the supplied static callback matches this connection.
     *
     * \param   callback passes the static function to compare.
     *
     * \return  True if the function matches the stored callback.
     */
    bool isCallback(const StaticCallback_t& callback) const {
        return callback == _staticCallback;
    }

private:
    /**
     * \brief   Non-owning pointer to the object that emits the signal.
     */
    MetaObject* _sender;

    /**
     * \brief   Non-owning pointer to the receiver, or \c nullptr for a static callback.
     */
    MetaObject* _receiver;

    /**
     * \brief   Signal member function associated with this connection.
     */
    const Signal_t _signal;

    /**
     * \brief   Receiver callback, or \c nullptr when using a static callback.
     */
    const Callback_t _callback;

    /**
     * \brief   Static callback, or \c nullptr when using a receiver callback.
     */
    const StaticCallback_t _staticCallback;
};

}

#endif  //__MERCURYCAN_SUPPORT_SIGNAL_SLOT_CONNECTION_HPP__