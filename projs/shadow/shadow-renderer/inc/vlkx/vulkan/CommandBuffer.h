#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class CommandBuffer {
public:

	CommandBuffer();
	~CommandBuffer();

	VkCommandPool commands;
	std::vector<VkCommandBuffer> buffers;

	void createCommandPoolAndBuffers(size_t images);
	void beginCommandBuffer(VkCommandBuffer buffer);
	void endCommandBuffer(VkCommandBuffer buffer);

	void createCommandPool();
	void allocateCommandBuffers(size_t size);

	void destroy();
};