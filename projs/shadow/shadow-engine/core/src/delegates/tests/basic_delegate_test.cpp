//
// Created by dpete on 27/09/2022.
//


#include "delegates/event_bus.h"

void test(){

    EventBus bus;

    bus.subscribe<TestEvent>([](TestEvent& e){});

}

