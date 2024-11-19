#pragma once

#include <shadow/SHObject.h>
#include <shadow/event-bus/events.h>

namespace SH::Renderer {

  class LateRender : public Events::Event {
    SHObject_Base(LateRender)
  };

}