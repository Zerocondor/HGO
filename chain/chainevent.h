#ifndef __HGO_CHAINEVENT__
#define __HGO_CHAINEVENT__
#include <functional>
#include <memory>
#include <deque>
#include <vector>
#include <thread>
#include "../token/transaction.h"

namespace HGO::CHAIN::EVENTS
{

    class ChainEvent
    {
        constexpr static char const * EVENT_NAME = "CHAIN_EVENT";
        public:
            ChainEvent() = default;
            virtual std::string eventName() const;
            virtual const std::string data() const;
            virtual ~ChainEvent() = 0;
    };

    class NewBlockEvent : public ChainEvent
    {
        constexpr static char const * EVENT_NAME = "NEW_BLOCK";
        public:
            NewBlockEvent() = delete;
            NewBlockEvent(const Block &block);

            virtual std::string eventName() const override;
            virtual const std::string data() const override;
            virtual ~NewBlockEvent() override;
        protected:
            const Block _block;
    };

    class NewTransactionEvent : public ChainEvent
    {
        constexpr static char const * EVENT_NAME = "NEW_TRANSACTION";
        public:
            NewTransactionEvent() = delete;
            NewTransactionEvent(const HGO::TOKEN::Transaction &tx);
            virtual std::string eventName() const override;
            virtual const std::string data() const override;
            virtual ~NewTransactionEvent() override;
        protected:
            const HGO::TOKEN::Transaction _tx;
    };

    class ChainEventManager
    {
        public:
            using EVENT_CALLBACK = std::function<bool(const ChainEvent&)>;
            using EVENT_HANDLER = std::vector<EVENT_CALLBACK>;
            using HANDLER_REF = EVENT_HANDLER::size_type;
        protected:
            using EVENT_BUFFER = std::deque<std::shared_ptr<ChainEvent>>;

        public:
            ChainEventManager();
            ChainEventManager(ChainEventManager && o);
            ~ChainEventManager();

            ChainEventManager &operator=(ChainEventManager && other);

            void run();
            template<typename T>
            bool dispatchEvent(const T &event);
            HANDLER_REF registerCallback(EVENT_CALLBACK cb);
            bool unregisterCallback(const HANDLER_REF &idx);

        protected:
            void _dispatchImpl();
            bool _running;
            std::thread _threadDispatcher;
            EVENT_HANDLER _registeredCallback;
            EVENT_BUFFER _eventBuffer;

    };

    template<typename T>
    bool ChainEventManager::dispatchEvent(const T &event)
    {
        _eventBuffer.push_front(std::shared_ptr<ChainEvent>(new T(event)));
        return true;
    }
}


#endif