#pragma once
#if false

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

    /**
     * Helper template for any code that wants to narrow for only Events
     * @tparam T
     */
    template<typename T>
    concept EventType = std::is_base_of<T, SH::Events::Event>::value;

    struct Subscription {
        using Callback = std::function<const void(SH::Events::Event &)>;

        Callback callback;

        std::weak_ptr<ShadowEngine::SHObject> binding;

        explicit Subscription(Callback call): callback(call){}
    };

    /**
     * Interface class for all event dispatcher implementations
     */
    class EventDispatcherBase {
    public:
        using SubRef = std::shared_ptr<Subscription>;

        virtual ~EventDispatcherBase() = default;

        virtual void call(ShadowEngine::SHObject &obj) = 0;
    };

    template<EventType T>
    class EventDispatcherSingleThread : public EventDispatcherBase {
    protected:
        using SubscriptionList = std::vector<SubRef>;

        SubscriptionList subscriptionList;

    public:
        std::shared_ptr<Subscription> subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<const void(T &)> func) {
            auto sub = std::make_shared<Subscription>(func);
            subscriptionList.push_back(sub);
            return sub;
        }

        void unsubscribe(const std::shared_ptr<Subscription> ref) {
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
    class EventDispatcherThreaded : public EventDispatcherSingleThread<T> {
        std::mutex mObserversMutex;

    public:

        void call(ShadowEngine::SHObject &obj) override {
            std::lock_guard<std::mutex> lLockGuard(mObserversMutex);

            this->call_impl((T&)obj);
        }
    };


    /**
     * EventBus holds Dispatchers for all event types that are used
     * It routes the subscriptions, un-subscriptions and event publishing
     * to the handling dispatchers.
     */
    class EventBus {

        template<class T,
                class V =
                std::conditional<
                        std::is_base_of<Event,T>::value,
                        EventDispatcherSingleThread<T>,
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
        EventDispatcherBase::SubRef subscribe(const std::shared_ptr<ShadowEngine::SHObject> &binding, std::function<void(T &)> func) {
            auto dis = getDispatcher<T>();

            return dis->subscribe(binding, func);
        }

        template<EventType T>
        void unsubscribe(EventDispatcherBase::SubRef ref) {

            auto dis = getDispatcher<T>();
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

#endif