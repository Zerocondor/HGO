#ifndef __HGO_CHAINEVENT__
#define __HGO_CHAINEVENT__
#include <functional>
#include <memory>
#include <deque>
#include <vector>
#include <thread>

namespace HGO::CHAIN::EVENTS
{

    class ChainEvent
    {
        constexpr static char const * EVENT_NAME = "CHAIN_EVENT";
        public:
            ChainEvent() = default;
            virtual std::string eventName() const;
            virtual ~ChainEvent() = 0;
    };

    class NewBlockEvent : public ChainEvent
    {
        constexpr static char const * EVENT_NAME = "NEW_BLOCK";
        public:
            NewBlockEvent() = default;
            virtual std::string eventName() const override;
            virtual ~NewBlockEvent() override;
    };

    class NewTransactionEvent : public ChainEvent
    {
        constexpr static char const * EVENT_NAME = "NEW_TRANSACTION";
        public:
            NewTransactionEvent() = default;
            virtual std::string eventName() const override;
            virtual ~NewTransactionEvent() override;
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