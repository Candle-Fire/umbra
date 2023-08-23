#pragma once

#include <SDL_events.h>

#include "SHObject.h"

namespace SH::Events {

    class Event : public ShadowEngine::SHObject {
      SHObject_Base(Event)
    };

    class SDLEvent : public Event {
      SHObject_Base(SDLEvent)
      public:
        SDLEvent(SDL_Event e) : event(e) {};

        SDL_Event event;
    };

    class OverlayRender : public Event {
      SHObject_Base(OverlayRender)
    };

    class Render : public Event {
      SHObject_Base(Render)
    };

    class Recreate : public Event {
      SHObject_Base(Recreate)
    };

    class EditorRender : public Event {
        SHObject_Base(EditorRender);
    };
}