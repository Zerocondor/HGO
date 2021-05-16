#include "../exceptions.h"
#include "chainevent.h"


using namespace HGO::CHAIN::EVENTS;

void ChainEvent::eventName() const{
    std::cout<<EVENT_NAME;
}

void NewBlockEvent::eventName() const{
    std::cout<<EVENT_NAME;
}
void NewTransactionEvent::eventName() const{
    std::cout<<EVENT_NAME;
}

ChainEvent::~ChainEvent() {}
NewBlockEvent::~NewBlockEvent(){}
NewTransactionEvent::~NewTransactionEvent(){}



/***
 * Event Manager
 */
ChainEventManager::ChainEventManager() 
: _running(false)
{}

ChainEventManager::ChainEventManager(ChainEventManager && o)
{
    _running = o._running;
    _threadDispatcher = std::move(o._threadDispatcher);
    _registeredCallback = std::move(o._registeredCallback);
    _eventBuffer = std::move(o._eventBuffer);
}

ChainEventManager::~ChainEventManager()
{
    _running = false;
    if(_threadDispatcher.joinable()) {
        _threadDispatcher.join();
    }
}

void ChainEventManager::run()
{
    if(!_running)
    {
        _running = true;
        _threadDispatcher = std::thread(std::function<void(ChainEventManager *)>(&ChainEventManager::_dispatchImpl), this);
    }
}

ChainEventManager::HANDLER_REF ChainEventManager::registerCallback(EVENT_CALLBACK cb)
{
    _registeredCallback.push_back(cb);
    return _registeredCallback.size() - 1;
}
bool ChainEventManager::unregisterCallback(const HANDLER_REF &idx)
{
    if(idx >= _registeredCallback.size())
        return false;

    _registeredCallback.erase(_registeredCallback.begin() + idx); 
    return true;   
}
void ChainEventManager::_dispatchImpl() 
{
    while(_running)
    {
        if(_eventBuffer.size()) {
            std::shared_ptr<ChainEvent> evt = _eventBuffer.back();
            EVENT_HANDLER copyHandlers = _registeredCallback; //Thread Safe avoid removing dispatcher while iterating
            for(EVENT_CALLBACK cb : copyHandlers)
            {
                cb(*evt);
            }
            _eventBuffer.pop_back();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}