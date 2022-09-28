#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>

#include "SHObject.h"

template<class T, class U>
concept Derived = std::is_base_of<U, T>::value;

template<typename T>
concept SHObjectConcept = Derived<ShadowEngine::SHObject,T>;

class Event : public ShadowEngine::SHObject{
    SHObject_Base(Event)
};
SHObject_Base_Impl(Event)

class TestEvent : public ShadowEngine::SHObject{
SHObject_Base(TestEvent)
};
SHObject_Base_Impl(TestEvent)

class EventDispatcherBase{
public:
    virtual void subscribe(std::function<const void(Event&)> func) = 0;

};

template<class T>
class EventDispatcher : public EventDispatcherBase{

    using RecieverList = std::map<std::string, std::function<const void(T&)>>;

public:
    RecieverList list;

    void subscribe(std::function<const void(Event &)> func) override {

    }

    void subscribe(std::function<const void(T &)> func) {

    }
};

class EventBus{

public:

    std::map<int, std::shared_ptr<EventDispatcherBase>> dispatchers;

    template<Derived<ShadowEngine::SHObject> T>
    void subscribe(std::function<const void(T&)> func){

        if(dispatchers.contains(T::TypeId())) {
            auto dis = std::reinterpret_pointer_cast<EventDispatcher<T>>(dispatchers[T::TypeId()]);
            dis->subscribe(func);
        }
    }

};


