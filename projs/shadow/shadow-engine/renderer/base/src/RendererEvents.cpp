#include "shadow/event-bus/event_bus.h"
#include "shadow/event-bus/events.h"
#include "shadow/SHObject.h"

#include "shadow/renderer/RendererEvents.h"

namespace SH::Renderer {

  SHObject_Base_Impl(LateRender)

  Event_Impl(LateRender)
}