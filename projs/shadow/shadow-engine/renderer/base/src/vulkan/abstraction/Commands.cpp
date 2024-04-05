#include "shadow/renderer/vlkx/vulkan/abstraction/Commands.h"
#include "shadow/renderer/vlkx/vulkan/VulkanModule.h"

#include <limits>

vlkx::CommandBuffer::CommandBuffer() {
    dev = VulkanModule::getInstance()->getDevice();
}

VkCommandPool createPool(vlkx::Queue queue, bool shortLived) {
    VkCommandPool pool;
    VkCommandPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.queueFamilyIndex = queue.queueIndex;
    poolCreateInfo.flags = shortLived ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // Start the pool
    if (vkCreateCommandPool(VulkanModule::getInstance()->getDevice()->logical, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate a temporary command pool");

    return pool;
}

std::vector<VkCommandBuffer> allocateBuffers(const VkCommandPool& pool, uint32_t amount) {
    const VkCommandBufferAllocateInfo allocate {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, amount
    };

    std::vector<VkCommandBuffer> buffers(amount);
    if (vkAllocateCommandBuffers(VulkanModule::getInstance()->getDevice()->logical, &allocate, buffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate command buffers");

    return buffers;
}

void recordCommands(const VkCommandBuffer& commands, VkCommandBufferUsageFlags flags, const vlkx::CommandBuffer::Command& record) {
    const VkCommandBufferBeginInfo begin {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, flags, nullptr
    };

    if (vkBeginCommandBuffer(commands, &begin) != VK_SUCCESS)
        throw std::runtime_error("Unable to begin listening on command buffer");
    if (record != nullptr)
        record(commands);
    if (vkEndCommandBuffer(commands) != VK_SUCCESS)
        throw std::runtime_error("Unable to end listening on command buffer");
}

std::optional<VkResult> parseResult(VkResult result) {
    switch(result) {
        case VK_ERROR_OUT_OF_DATE_KHR: return result;
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            return std::nullopt;

        default:
            throw std::runtime_error("Command buffer returned unknown result " + std::to_string(result));
    }
}

vlkx::ImmediateCommand::ImmediateCommand(Queue queue) : queue(queue) {
    const auto pool = createPool(queue, true);
    setPool(pool);
    commands = allocateBuffers(pool, 1)[0];
}

void vlkx::ImmediateCommand::run(const vlkx::CommandBuffer::Command &cmd) {
    recordCommands(commands, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, cmd);
    const VkSubmitInfo submit {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr, 0, nullptr, nullptr, 1,
        &commands, 0, nullptr
    };

    vkQueueSubmit(queue.queue, 1, &submit, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue.queue);
}

vlkx::RenderCommand::RenderCommand(int frames) {
    VulkanDevice* dev = VulkanModule::getInstance()->getDevice();

    // Start semaphores for render events
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(dev->logical, &semaphoreInfo, nullptr, &newImageSem);
    vkCreateSemaphore(dev->logical, &semaphoreInfo, nullptr, &renderDoneSem);

    // Start fences for the frames
    inFlight.resize(frames);
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < frames; i++) {
        if (vkCreateFence(dev->logical, &fenceInfo, nullptr, &inFlight[i]) != VK_SUCCESS)
            throw std::runtime_error("Unable to create fence for a frame");
    }

    const auto pool = createPool({ dev->graphicsQueue, dev->queueData.graphics }, false);
    setPool(pool);
    commands = allocateBuffers(pool, static_cast<uint32_t>(frames));
}

std::optional<VkResult> vlkx::RenderCommand::execute(int frame, const VkSwapchainKHR &swapchain,
                                                     const vlkx::RenderCommand::Update &update,
                                                     const vlkx::RenderCommand::Command &cmd) {

    const VkDevice& logical = VulkanModule::getInstance()->getDevice()->logical;

    vkWaitForFences(logical, 1, &inFlight[imageIndex], VK_TRUE,
                    std::numeric_limits<uint64_t>::max()
    );

    if (update != nullptr)
        update(frame);

    uint32_t nextFrame;
    // Prepare for a new frame to start
    const auto result = parseResult(vkAcquireNextImageKHR(logical, swapchain,
                          std::numeric_limits<uint64_t>::max(), newImageSem, VK_NULL_HANDLE, &nextFrame
    ));

    if (result.has_value())
        return result;

    recordCommands(commands[imageIndex], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, [&cmd, nextFrame](const VkCommandBuffer& buffer) {
        cmd(buffer, nextFrame);
    });

    constexpr VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr, 1, &newImageSem, &stage, 1, &commands[imageIndex], 1, &renderDoneSem
    };

    vkResetFences(logical, 1, &inFlight[imageIndex]);

    if (VkResult res = vkQueueSubmit(VulkanModule::getInstance()->getDevice()->graphicsQueue, 1, &submit, inFlight[imageIndex]); res != VK_SUCCESS)
        throw std::runtime_error("Failed to submit commands: " + std::to_string(res));

    const VkPresentInfoKHR present {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr, 1, &renderDoneSem, 1, &swapchain, &nextFrame, nullptr
    };

    return parseResult(vkQueuePresentKHR(VulkanModule::getInstance()->getDevice()->presentationQueue, &present));
}


std::optional<VkResult> vlkx::RenderCommand::executeSimple(int frame,
                                                     const vlkx::RenderCommand::Update &update,
                                                     const vlkx::RenderCommand::Command &cmd) {
    if (update != nullptr)
        update(frame);

    const VkDevice& logical = VulkanModule::getInstance()->getDevice()->logical;

    vkWaitForFences(logical, 1, &inFlight[imageIndex], VK_TRUE,
                    std::numeric_limits<uint64_t>::max()
    );

    recordCommands(commands[imageIndex], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, [&cmd](const VkCommandBuffer& buffer) {
        cmd(buffer, 0);
    });

    vkResetFences(logical, 1, &inFlight[imageIndex]);

    constexpr VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    const VkSubmitInfo submit {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            nullptr, 0, VK_NULL_HANDLE, &stage, 1, &commands[imageIndex], 0, VK_NULL_HANDLE
    };

    return std::make_optional(vkQueueSubmit(VulkanModule::getInstance()->getDevice()->graphicsQueue, 1, &submit, inFlight[imageIndex]));
}

