#ifndef __META_OBJECT_HPP__
#define __META_OBJECT_HPP__

#include "../list/container.hpp"
#include <type_traits>

#include "connection.hpp"

namespace BareSignal 
{

#define emit
#define signals public

class MetaObject
{
public:
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

    template<class Sender,
             class SenderBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender>
    static void connect(const Sender* sender,
                        void(SenderBase::*signal)(ParamPack...),
                        void(*callback)(ParamPack...)
    ) {
        Connection<ParamPack...>* connection =
            new Connection<ParamPack...>(
                sender,
                signal,
                callback
            );

        _connections<ParamPack...>.push_back(connection);
    }

    template<class Sender,
             class SenderBase,
             class Receiver,
             class ReceiverBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender> &&
             isDerived<ReceiverBase, Receiver>
    static void disconnect(const Sender* sender,
                           void(SenderBase::*signal)(ParamPack...),
                           const Receiver* receiver,
                           void(ReceiverBase::*callback)(ParamPack...)
    ) {
        auto& connections = _connections<ParamPack...>;

        for (uint8_t i = 0; i < connections.length(); i++) {
            Connection<ParamPack...>* connection = connections.at(i);

            if (connection &&
                connection->isSender(static_cast<const MetaObject*>(sender)) &&
                connection->isReceiver(static_cast<const MetaObject*>(receiver)) &&
                connection->isSignal(static_cast<Connection<ParamPack...>::Signal_t>(signal)) &&
                connection->isCallback(static_cast<Connection<ParamPack...>::Callback_t>(callback))) {
                delete connection;
                connections.remove(connection);
                return;
            }
        }
    }

    template<class Sender,
             class SenderBase,
             class... ParamPack>
    requires isDerived<SenderBase, Sender>
    static void disconnect(const Sender* sender,
                           void(SenderBase::*signal)(ParamPack...),
                           void(*callback)(ParamPack...)
    ) {
        auto& connections = _connections<ParamPack...>;

        for (uint8_t i = 0; i < connections.length(); i++) {
            Connection<ParamPack...>* connection = connections.at(i);

            if (connection &&
                connection->isSender(static_cast<const MetaObject*>(sender)) &&
                connection->isSignal(static_cast<Connection<ParamPack...>::Signal_t>(signal)) &&
                connection->isCallback(callback)) {
                delete connection;
                connections.remove(connection);
                return;
            }
        }
    }

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
    template<class... ParamPack>
    inline static Generic::Container<Connection<ParamPack...>*> _connections;
};

}

#endif //__META_OBJECT_HPP__