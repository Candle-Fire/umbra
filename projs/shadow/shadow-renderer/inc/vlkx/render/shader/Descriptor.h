#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Descriptor {
public:
	Descriptor();
	~Descriptor();

	// Global descriptor bindings
	VkDescriptorSetLayout layout;
	VkDescriptorPool pool;
	VkDescriptorSet set;

	// Allocate and prepare the descriptors for a simple (uniform buffer only) descriptor.
	void createAndAllocateSimple(uint32_t imageCount);

	// Fill the Descriptors with the given uniforms
	void populate(uint32_t imageCount, VkBuffer uniforms, size_t bufferSize);
	void destroy();

private:

	// Set up the layout for a single UBO
	void createSimpleLayout();
	// Setup the pool for a single UBO
	void createSimplePool(uint32_t imageCount);

};