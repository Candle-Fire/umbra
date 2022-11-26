#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class RenderPass {
public:
	RenderPass();
	~RenderPass();

	VkRenderPass pass;

	void createVertexRenderPass(VkFormat format);
	void createRTRenderPass(VkFormat format);
	void createRTPhysicsPass(VkFormat format);

	void beginRenderPass(std::vector<VkClearValue> clearValues, VkCommandBuffer commands, VkFramebuffer framebuffer, VkExtent2D extent);
	void endRenderPass(VkCommandBuffer commands);

	void destroy();
};