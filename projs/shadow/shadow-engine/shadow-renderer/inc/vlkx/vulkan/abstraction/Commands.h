#pragma once

#include <vulkan/vulkan.h>
#include "Queue.h"
#include "vlkx/vulkan/VulkanDevice.h"
#include <functional>
#include <optional>

namespace vlkx {

    // Root class of VkCommandBuffer wrappers.
    class CommandBuffer {
    public:
        using Command = std::function<void(const VkCommandBuffer& buffer)>;

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        virtual ~CommandBuffer() {
            vkDestroyCommandPool(dev->logical, pool, nullptr);
        }
    protected:
        CommandBuffer();

        void setPool(const VkCommandPool& newPool) { pool = newPool; }
        VulkanDevice* dev;
    private:
        VkCommandPool pool;
    };

    // A command buffer that will be immediately executed.
    class ImmediateCommand : public CommandBuffer {
    public:
        ImmediateCommand(Queue queue);

        ImmediateCommand(const ImmediateCommand&) = delete;
        ImmediateCommand& operator=(const ImmediateCommand&) = delete;

        void run(const Command& cmd);

    private:
        const Queue queue;
        VkCommandBuffer commands;
    };

    // A command buffer that will be reused every frame.
    class RenderCommand : public CommandBuffer {
    public:
        using Command = std::function<void(const VkCommandBuffer& commands, uint32_t framebuffer)>;
        using Update = std::function<void(int frame)>;

        ~RenderCommand() override {
            // Destroy our own data
            vkDestroySemaphore(dev->logical, renderDoneSem, nullptr);
            vkDestroySemaphore(dev->logical, newImageSem, nullptr);

            for (size_t i = 0; i < 2; i++) {
                vkDestroyFence(dev->logical, inFlight[i], nullptr);
            }
        }

        RenderCommand(int frames);

        RenderCommand(const RenderCommand&) = delete;
        RenderCommand& operator=(const RenderCommand&) = delete;

        uint32_t getFrame() { return imageIndex; }

        void nextFrame() { imageIndex = (imageIndex + 1) % 2; }

        std::optional<VkResult> execute(int frame, const VkSwapchainKHR& swapchain, const Update& update, const Command& cmd);
        // Renders a single frame out, no semaphores or fences.
        std::optional<VkResult> executeSimple(int frame, const Update& update, const Command& cmd);

    private:
        std::vector<VkCommandBuffer> commands;
        // Raised when a new image is available
        VkSemaphore newImageSem;
        // Raised when a render is finished
        VkSemaphore renderDoneSem;
        // Stores fences for frames that are currently "in flight".
        std::vector<VkFence> inFlight;

        // The index of the texture that is currently being used by the GPU.
        uint32_t imageIndex = 0;
    };
}