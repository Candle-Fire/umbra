#include "shadow/event-bus/event_bus.h"
#include "shadow/SHObject.h"

namespace SH::Events {

    SHObject_Base_Impl(Event)

    SHObject_Base_Impl(SDLEvent)

    Event_Impl(SDLEvent)

    SHObject_Base_Impl(Recreate)

    Event_Impl(Recreate)

    SHObject_Base_Impl(ImGui)

    Event_Impl(ImGui)

    SHObject_Base_Impl(RenderGeometry)

    Event_Impl(RenderGeometry)

}