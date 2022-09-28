#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <any>
#include <SDL_events.h>

#include "SHObject.h"

namespace ShadowEngine {

    template<class T, class U>
    concept Derived = std::is_base_of<T, U>::value;

    template<typename T>
    concept SHObjectConcept = Derived<ShadowEngine::SHObject, T>;

    class Event : public ShadowEngine::SHObject {
    SHObject_Base(Event)
    };


    class SDLEvent : public Event {
    SHObject_Base(SDLEvent)
    public:
        SDLEvent(SDL_Event e) : event(e) {};

        SDL_Event event;
    };


    class TestEvent : public Event {
    SHObject_Base(TestEvent)
    };


    template<typename T>
    concept EventType = Derived<Event, T>;

    template<EventType T>
    struct Subscription {
        std::function<const void(T &)> callback;

        std::weak_ptr<ShadowEngine::SHObject> binding;
    };

    class EventDispatcherBase {
    public:
        virtual void call(ShadowEngine::SHObject &obj) = 0;
    };

    template<EventType T>
    class EventDispatcher : public EventDispatcherBase {

        using RecieverList = std::vector<Subscription<T>>;

    public:
        RecieverList list;

        void subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<const void(T &)> func) {
            list.push_back(Subscription<T>{.callback = func, .binding= binding});
        }

        void call(ShadowEngine::SHObject &obj) override {
            this->call_impl((T&)obj);
        }

        void call_impl(T &event) {
            for (int i = 0; i < list.size(); ++i) {
                list[i].callback(event);
            }
        }
    };

    class EventBus {

    public:

        std::map<uint64_t, std::shared_ptr<EventDispatcherBase>> dispatchers;

        template<EventType T>
        void subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<void(T &)> func) {

            std::shared_ptr<EventDispatcher<T>> dis;

            if (dispatchers.contains(T::TypeId())) {
                dis = std::reinterpret_pointer_cast<EventDispatcher<T>>(dispatchers[T::TypeId()]);
            } else {
                dis = std::make_shared<EventDispatcher<T>>();
                dispatchers.emplace(T::TypeId(), dis);
            }

            dis->subscribe(binding, func);
        }

        void fire(Event &e) {
            auto id = e.GetTypeId();
            if (dispatchers.contains(id)) {
                dispatchers[id]->call(e);
            }
        }

    };
}