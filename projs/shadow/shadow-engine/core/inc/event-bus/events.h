#pragma once

#include <SDL_events.h>

#include "SHObject.h"

namespace ShadowEngine {

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

}