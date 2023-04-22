#pragma once

#include <functional>
#include <memory>

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

        std::weak_ptr<ShadowEngine::SHObject> binding;

        explicit Subscription(Callback<T> call) : callback(call) {}
    };

    using BusID = int;

    constexpr BusID MainBus = 0;

    template<EventType T, BusID Bus = SH::Events::MainBus>
    class EventDispatcher {
      public:
        using SubRef = Subscription<T>;
        using SubscriptionList = std::vector<SubRef>;

        static SubscriptionList subscriptionList;

        static void subscribe(std::function<const void(T &)> func) {
            //auto sub = std::make_shared<Subscription>(func);
            subscriptionList.push_back(Subscription(func));
        }

        static void call(T &event) {
            for (int i = 0; i < subscriptionList.size(); ++i) {
                subscriptionList[i].callback(event);
            }
        }
    };

    template<EventType T, BusID Bus>
    EventDispatcher<T, Bus>::SubscriptionList EventDispatcher<T, Bus>::subscriptionList;

}