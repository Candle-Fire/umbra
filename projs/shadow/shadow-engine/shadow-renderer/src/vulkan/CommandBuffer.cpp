#include <vlkx/vulkan/CommandBuffer.h>
#include <vlkx/vulkan/VulkanManager.h>

CommandBuffer::CommandBuffer() {}
CommandBuffer::~CommandBuffer() {}

void CommandBuffer::createCommandPoolAndBuffers(size_t size) {
	createCommandPool();
	allocateCommandBuffers(size);
}

void CommandBuffer::createCommandPool() {
	// Prepare queues
	QueueFamilies families = VulkanManager::getInstance()->getDevice()->getQueues();

	// Prepare command pool creation data
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = families.graphics;
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	// Attempt creation
	if (vkCreateCommandPool(VulkanManager::getInstance()->getDevice()->logical, &info, nullptr, &commands) != VK_SUCCESS)
		throw std::runtime_error("Unable to create a command pool.");
}

void CommandBuffer::allocateCommandBuffers(size_t size) {
	// Prepare allocation info
	buffers.resize(size);

	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = commands;
	info.commandBufferCount = (uint32_t)buffers.size();

	// Attempt allocation
	if (vkAllocateCommandBuffers(VulkanManager::getInstance()->getDevice()->logical, &info, buffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate command buffer");
}

void CommandBuffer::beginCommandBuffer(VkCommandBuffer buffer) {
	// Prepare to begin listening on this buffer
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // Allow writing to this buffer while another is being read.

	// Attempt to begin listening
	if (vkBeginCommandBuffer(buffer, &info) != VK_SUCCESS)
		throw std::runtime_error("Unable to begin a command buffer");
}

void CommandBuffer::endCommandBuffer(VkCommandBuffer buffer) {
	// Attempt to end listening
	if (vkEndCommandBuffer(buffer) != VK_SUCCESS)
		throw std::runtime_error("Unable to end listening for a command buffer");
}

void CommandBuffer::destroy() {
	vkDestroyCommandPool(VulkanManager::getInstance()->getDevice()->logical, commands, nullptr);
}
