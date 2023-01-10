#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <any>

#include "SHObject.h"
#include "events.h"

/**
 * Base types for the module event dispatcher
 */

namespace ShadowEngine::EventBus {

    template<class T, class U>
    concept Derived = std::is_base_of<T, U>::value;

    template<typename T>
    concept SHObjectConcept = Derived<ShadowEngine::SHObject, T>;




    template<typename T>
    concept EventType = Derived<Event, T>;

    template<EventType T>
    struct Subscription {
        using Callback = std::function<const void(T &)>;

        Callback callback;

        std::weak_ptr<ShadowEngine::SHObject> binding;

        explicit Subscription(Callback call): callback(call){}
    };

    /**
     * Base class for calling all subscribers to an event
     */
    class EventDispatcherBase {
    public:
        virtual ~EventDispatcherBase() = default;

        virtual void call(ShadowEngine::SHObject &obj) = 0;
    };

    template<EventType T>
    class EventDispatcher : public EventDispatcherBase {
    public:
        using SubRef = std::shared_ptr<Subscription<T>>;

    protected:
        using SubscriptionList = std::vector<SubRef>;

        SubscriptionList subscriptionList;

    public:
        std::shared_ptr<Subscription<T>> subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<const void(T &)> func) {
            auto sub = std::make_shared<Subscription<T>>(func);
            subscriptionList.push_back(sub);
            return sub;
        }

        void unsubscribe(const std::shared_ptr<Subscription<T>> ref) {
            subscriptionList.erase(std::remove(subscriptionList.begin(), subscriptionList.end(), ref), subscriptionList.end());
        }

        void call(ShadowEngine::SHObject &obj) override {
            this->call_impl((T&)obj);
        }

        void call_impl(T &event) {
            for (int i = 0; i < subscriptionList.size(); ++i) {
                subscriptionList[i]->callback(event);
            }
        }
    };

    template<EventType T>
    class EventDispatcherThreaded : public EventDispatcher<T> {
        std::mutex mObserversMutex;

    public:

        void call(ShadowEngine::SHObject &obj) override {
            std::lock_guard<std::mutex> lLockGuard(mObserversMutex);

            this->call_impl((T&)obj);
        }
    };

    class EventBus {

        template<class T,
                class V =
                std::conditional<
                        std::is_base_of<Event,T>::value,
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
        EventDispatcher<T>::SubRef subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<void(T &)> func) {

            std::shared_ptr<EventDispatcher<T>> dis;

            dis = getDispatcher<T>();

            return dis->subscribe(binding, func);
        }

        template<EventType T>
        void unsubscribe(EventDispatcher<T>::SubRef ref) {

            std::shared_ptr<EventDispatcher<T>> dis = getDispatcher<T>();
            dis->unsubscribe(ref);
        }

        void fire(Event &e) {
            auto id = e.GetTypeId();
            if (dispatchers.contains(id)) {
                dispatchers[id]->call(e);
            }
        }

    };
}