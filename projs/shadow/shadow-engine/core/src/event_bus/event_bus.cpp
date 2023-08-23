#include "event-bus/event_bus.h"
#include "SHObject.h"

namespace SH::Events {

    SHObject_Base_Impl(Event)

    SHObject_Base_Impl(SDLEvent)

    Event_Impl(SDLEvent)

    SHObject_Base_Impl(OverlayRender)

    Event_Impl(OverlayRender)

    SHObject_Base_Impl(Recreate)

    Event_Impl(Recreate)

    SHObject_Base_Impl(EditorRender)

    Event_Impl(EditorRender)

    SHObject_Base_Impl(Render)

    Event_Impl(Render)
}