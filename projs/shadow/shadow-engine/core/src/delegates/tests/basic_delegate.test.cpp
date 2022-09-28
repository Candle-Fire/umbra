//
// Created by dpete on 27/09/2022.
//


#include "delegates/event_bus.h"

#include "debug/DebugModule.h"

void test(){

    ShadowEngine::EventBus bus;

    auto bind = std::make_shared<ShadowEngine::Debug::DebugModule>();

    bus.subscribe<ShadowEngine::TestEvent>(bind,[](ShadowEngine::TestEvent& e){});

}

