#include "event-bus/event_bus.h"
#include "event-bus/render_events.h"

namespace SH::Events {

    SHObject_Base_Impl(PreRender)

    Event_Impl(PreRender)

    SHObject_Base_Impl(Render)

    Event_Impl(Render)

    SHObject_Base_Impl(PostRender)

    Event_Impl(PostRender)

}