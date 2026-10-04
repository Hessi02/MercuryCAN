/**
 * \file    metaObject.hpp
 * \brief   Defines the signal and callback registration interface.
 *
 * MetaObject allows objects to connect signals to member or static callbacks,
 * disconnect those connections, and execute callbacks associated with an
 * emitted signal.
 *
 * \copyright Copyright (C) 2026 Luca Hesselbrock
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __MERCURYCAN_SUPPORT_SIGNAL_SLOT_META_OBJECT_HPP__
#define __MERCURYCAN_SUPPORT_SIGNAL_SLOT_META_OBJECT_HPP__

#include <MercuryCAN/support/list/container.hpp>
#include <MercuryCAN/support/signalSlot/connection.hpp>

#include <type_traits>

namespace SignalSlot
{

#define emit
#define signals public

/**
 * \brief   Provides static signal-to-callback connection management.
 *
 * Connections are stored in static containers grouped by their parameter
 * types. Signals are ordinary member functions; the \c signals and \c emit
 * macros provide syntax compatible with signal/slot-style call sites.
 */
class MetaObject
{
public:
    /**
     * \brief   Connects a sender signal to a receiver member callback.
     *
     * Registers a connection that invokes \p callback on \p receiver whenever
     * the specified \p signal is emitted by \p sender. Sender and receiver
     * objects are not owned by the connection and must remain valid while the
     * connection may be executed.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \tparam  Receiver passes the concrete receiver type.
     * \tparam  ReceiverBase passes the base type declaring the callback.
     * \tparam  ParamPack passes the signal and callback parameter types.
     * \param   sender passes the object that emits the signal.
     * \param   signal passes the signal member function.
     * \param   receiver passes the object whose callback will be invoked.
     * \param   callback passes the receiver member function to invoke.
     */
    template<class Sender,
             class SenderBase,
             class Receiver,
             class ReceiverBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender> &&
             isDerived<ReceiverBase, Receiver>
    static void connect(const Sender* sender,
                        void(SenderBase::*signal)(ParamPack...),
                        const Receiver* receiver,
                        void(ReceiverBase::*callback)(ParamPack...)
    ) {
        Connection<ParamPack...>* connection = 
            new Connection<ParamPack...> (
                sender,
                signal,
                receiver,
                callback
            );
        
        _connections<ParamPack...>.push_back(connection);
    }

    /**
     * \brief   Connects a sender signal to a static callback function.
     *
     * Registers a connection that invokes \p callback whenever the specified
     * \p signal is emitted by \p sender.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \tparam  ParamPack passes the signal and callback parameter types.
     * \param   sender passes the object that emits the signal.
     * \param   signal passes the signal member function.
     * \param   callback passes the static function to invoke.
     */
    template<class Sender,
             class SenderBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender>
    static void connect(Sender* sender,
                        void(SenderBase::*signal)(ParamPack...),
                        void(*callback)(ParamPack...)
    ) {
        Connection<ParamPack...>* connection =
            new Connection<ParamPack...>(
                sender,
                signal,
                callback
            );

        _connections<ParamPack...>.append(connection);
    }

    /**
     * \brief   Disconnects a receiver member callback from a sender signal.
     *
     * Finds and removes the first connection matching all four supplied
     * objects and member functions. If no matching connection exists, nothing
     * is changed.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \tparam  Receiver passes the concrete receiver type.
     * \tparam  ReceiverBase passes the base type declaring the callback.
     * \tparam  ParamPack passes the signal and callback parameter types.
     * \param   sender passes the object that emits the signal.
     * \param   signal passes the signal member function.
     * \param   receiver passes the connected receiver object.
     * \param   callback passes the connected receiver member function.
     */
    template<class Sender,
             class SenderBase,
             class Receiver,
             class ReceiverBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender> &&
             isDerived<ReceiverBase, Receiver>
    static void disconnect(Sender* sender,
                           void(SenderBase::*signal)(ParamPack...),
                           Receiver* receiver,
                           void(ReceiverBase::*callback)(ParamPack...)
    ) {
        Generic::Container<Connection<ParamPack...>*>& connections = _connections<ParamPack...>;

        for (unsigned char i = 0; i < connections.length(); i++) {
            Connection<ParamPack...>* connection = connections.at(i);

            if (connection &&
                connection->isSender(static_cast<const MetaObject*>(sender)) &&
                connection->isReceiver(static_cast<const MetaObject*>(receiver)) &&
                connection->isSignal(static_cast<Connection<ParamPack...>::Signal_t>(signal)) &&
                connection->isCallback(static_cast<Connection<ParamPack...>::Callback_t>(callback))) 
            {
                delete connection;
                connections.remove(connection);
                return;
            }
        }
    }

    /**
     * \brief   Disconnects a static callback from a sender signal.
     *
     * Finds and removes the first connection matching the supplied sender,
     * signal, and callback. If no matching connection exists, nothing is
     * changed.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \tparam  ParamPack passes the signal and callback parameter types.
     * \param   sender passes the object that emits the signal.
     * \param   signal passes the signal member function.
     * \param   callback passes the connected static function.
     */
    template<class Sender,
             class SenderBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender>
    static void disconnect(Sender* sender,
                           void(SenderBase::*signal)(ParamPack...),
                           void(*callback)(ParamPack...)
    ) {
        Generic::Container<Connection<ParamPack...>*>& connections = _connections<ParamPack...>;

        for (unsigned char i = 0; i < connections.length(); i++) {
            Connection<ParamPack...>* connection = connections.at(i);

            if (connection &&
                connection->isSender(static_cast<const MetaObject*>(sender)) &&
                connection->isSignal(static_cast<Connection<ParamPack...>::Signal_t>(signal)) &&
                connection->isCallback(callback)) 
            {
                delete connection;
                connections.remove(connection);
                return;
            }
        }
    }

    /**
     * \brief   Executes callbacks connected to the specified sender signal.
     *
     * Iterates the connections registered for the argument types and invokes
     * callbacks whose sender and signal match. The supplied arguments are
     * forwarded to each matching callback.
     *
     * \tparam  Sender passes the concrete sender type.
     * \tparam  SenderBase passes the base type declaring the signal.
     * \tparam  ParamPack passes the signal and callback parameter types.
     * \param   sender passes the object that emitted the signal.
     * \param   signal passes the emitted signal member function.
     * \param   args passes the arguments forwarded to connected callbacks.
     */
    template<class Sender,
             class SenderBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender>
    static void executeAllCallbacks(Sender* sender,
                                    void(SenderBase::*signal)(ParamPack...),
                                    ParamPack... args
    ) {
        for (Connection<ParamPack...>* connection : _connections<ParamPack...>) {
            if (sender && connection &&
                connection->isSender(static_cast<MetaObject*>(sender)) &&
                connection->isSignal(static_cast<Connection<ParamPack...>::Signal_t>(signal))
            ) {
                connection->executeCallback(args...);
            }
        }
    }

private:
    /**
     * \brief   Connections grouped by their signal and callback parameter types.
     *
     * The containers and their connection records are shared by all
     * MetaObject instances.
     */
    template<class... ParamPack>
    inline static Generic::Container<Connection<ParamPack...>*> _connections;
};

}

#endif  //__MERCURYCAN_SUPPORT_SIGNAL_SLOT_META_OBJECT_HPP__