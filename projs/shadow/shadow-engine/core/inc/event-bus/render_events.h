#pragma once

#include <vulkan/vulkan.h>
#include "SHObject.h"

namespace SH::Events {

    // For any tasks that must come before / under geometry.
    // DO NOT RENDER GEOMETRY here, as it will conflict if the editor is enabled - causing a recursive loop.
    class PreRender : public Event {
        SHObject_Base(PreRender)
    };

    // Submit geometry to be rendered.
    class Render : public Event {
        SHObject_Base(Render)
    public:
        Render(const VkCommandBuffer &buffer, int frame) : buffer(buffer), frame(frame) {}

        VkCommandBuffer buffer;
        int frame;
    };

    // Post processing, and other deferred effects.
    class PostRender : public Event {
        SHObject_Base(PostRender);
    public:
        PostRender(const VkCommandBuffer &buffer, int frame) : buffer(buffer), frame(frame) {}

        VkCommandBuffer buffer;
        int frame;
    };
}