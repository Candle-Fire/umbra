#pragma once

#include <SDL3/SDL_events.h>

#include "shadow/SHObject.h"
#include "shadow/renderer/vulkan/vlkx/vulkan/abstraction/Buffer.h"

namespace SH::Events {

  class Event : public SH::SHObject {
  SHObject_Base(Event)
  };

  class SDLEvent : public Event {
  SHObject_Base(SDLEvent)
  public:
    SDLEvent(SDL_Event e) : event(e) {};

    SDL_Event event;
  };

  class Recreate : public Event {
  SHObject_Base(Recreate)
  };

  class ImGui : public Event
  {
    SHObject_Base(ImGui)
  };

  class RenderGeometry : public Event {
    SHObject_Base(RenderGeometry)

  public:
    RenderGeometry(uint32_t frame, VkCommandBuffer& buffer) : frame(frame), buffer(buffer) {}
    uint32_t frame;
    VkCommandBuffer& buffer;
  };
}
