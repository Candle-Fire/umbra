#include "event-bus/event_bus.h"
#include "SHObject.h"

namespace SH::Events {

    SHObject_Base_Impl(Event)

    SHObject_Base_Impl(SDLEvent)

    Event_Impl(SDLEvent)

    SHObject_Base_Impl(OverlayRender)

    Event_Impl(OverlayRender)

    SHObject_Base_Impl(PreRender)

    Event_Impl(PreRender)

    SHObject_Base_Impl(Recreate)

    Event_Impl(Recreate)

}