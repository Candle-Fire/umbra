#pragma once

#include <functional>
#include <memory>
#include <ranges>

#include "SHObject.h"
#include "events.h"

namespace SH::Events {

    template<typename T>
    concept EventType = std::is_base_of<Event, T>::value;

    template<EventType T>
    using Callback = std::function<const void(T &)>;

    template<EventType T>
    struct Subscription {
        Callback<T> callback;

        std::weak_ptr<SH::SHObject> binding;

        explicit Subscription(Callback<T> call) : callback(call) {}
    };

    using BusID = int;

    constexpr BusID MainBus = 0;

    template<EventType T>
    using SubRef = std::shared_ptr<Subscription<T>>;
    template<EventType T>
    using SubscriptionList = std::vector<SubRef<T>>;

    template<EventType T>
    class API EventDispatcherHolder {
      public:
        using BusMap = std::vector<SubscriptionList<T>>;
        static BusMap subscriptions;
    };

    #define Event_Impl(type) template<> API EventDispatcherHolder<type>::BusMap EventDispatcherHolder<type>::subscriptions(10);

    template<EventType T,
        BusID Bus = SH::Events::MainBus,
        class Holder = EventDispatcherHolder<T>>
    class EventDispatcher {
      public:

        static SubRef<T> subscribe(std::function<const void(T &)> func) {
            auto sub = std::make_shared<Subscription<T>>(func);
            Holder::subscriptions[Bus].push_back(sub);
            return sub;
        }

        void unsubscribe(const SubRef<T> ref) {
            auto &subList = Holder::subscriptions[Bus];
            subList.erase(std::remove(subList.begin(), subList.end(), ref), subList.end());
        }

        static void call(T &event) {
            auto &subList = Holder::subscriptions[Bus];
            for (int i = 0; i < subList.size(); ++i) {
                subList[i]->callback(event);
            }
        }
    };

    template<BusID id = SH::Events::MainBus>
    class EventBus {
        template<EventType T>
        using Dispatcher = EventDispatcher<T, id>;
      public:
        template<EventType T>
        SubRef<T> subscribe(std::function<const void(T &)> func) {
            return Dispatcher<T>::subscribe(func);
        }

        template<EventType T, typename Self>
        using MemberCallback = void (Self::*)(T &);

        template<EventType T, typename Self>
        SubRef<T> subscribe(Self *self, const MemberCallback<T, Self> &fn_ptr) {
            return Dispatcher<T>::subscribe(std::bind(fn_ptr, self, std::placeholders::_1));
        }

        template<EventType T>
        void fire(T &e) {
            Dispatcher<T>::call(e);
        }

        template<EventType T>
        void fire(T e) {
            Dispatcher<T>::call(e);
        }
    };

}