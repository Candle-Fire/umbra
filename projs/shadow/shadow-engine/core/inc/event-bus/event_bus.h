#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <any>

#include "SHObject.h"
#include "events.h"

namespace ShadowEngine {

    template<class T, class U>
    concept Derived = std::is_base_of<T, U>::value;

    template<typename T>
    concept SHObjectConcept = Derived<ShadowEngine::SHObject, T>;




    template<typename T>
    concept EventType = Derived<Event, T>;

    template<EventType T>
    struct Subscription {
        std::function<const void(T &)> callback;

        std::weak_ptr<ShadowEngine::SHObject> binding;
    };

    class EventDispatcherBase {
    public:
        virtual ~EventDispatcherBase() = default;

        virtual void call(ShadowEngine::SHObject &obj) = 0;
    };

    template<EventType T>
    class EventDispatcher : public EventDispatcherBase {

    protected:
        using SubscriptionList = std::vector<Subscription<T>>;

        SubscriptionList subscriptionList;
    public:

        void subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<const void(T &)> func) {
            subscriptionList.push_back(Subscription<T>{.callback = func, .binding= binding});
        }

        void call(ShadowEngine::SHObject &obj) override {
            this->call_impl((T&)obj);
        }

        void call_impl(T &event) {
            for (int i = 0; i < subscriptionList.size(); ++i) {
                subscriptionList[i].callback(event);
            }
        }
    };

    template<EventType T>
    class EventDispatcherThreaded : public EventDispatcher<T> {
        std::mutex mObserversMutex;

    public:

        void call(ShadowEngine::SHObject &obj) override {
            std::lock_guard<std::mutex> lLockGuard(mObserversMutex);

            this->subscriptionList.erase(
                    std::remove_if(
                            this->subscriptionList.begin(),
                            this->subscriptionList.end(),
                            [](const Subscription<T> i){
                                return i.binding.expired();
                            }
                    ),
                    this->subscriptionList.end());

            this->call_impl((T&)obj);
        }
    };

    class EventBus {

        template<class T,
                class V =
                std::conditional<
                        std::is_base_of<T,Event>::value,
                        EventDispatcher<T>,
                        EventDispatcherThreaded<T>
                >::type>
        std::shared_ptr<V> getDispatcher(){
            std::shared_ptr<V> dis;

            if (dispatchers.contains(T::TypeId())) {
                dis = std::reinterpret_pointer_cast<V>(dispatchers[T::TypeId()]);
            } else {
                dis = std::make_shared<V>();
                dispatchers.emplace(T::TypeId(), dis);
            }
            return dis;
        }

    public:

        std::map<uint64_t, std::shared_ptr<EventDispatcherBase>> dispatchers;

        template<EventType T>
        void subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<void(T &)> func) {

            std::shared_ptr<EventDispatcher<T>> dis;

            dis = getDispatcher<T>();

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