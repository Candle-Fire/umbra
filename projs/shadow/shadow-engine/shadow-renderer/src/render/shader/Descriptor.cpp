#include <vlkx/render/shader/Descriptor.h>
#include <vlkx/vulkan/VulkanManager.h>
#include <vlkx/render/Geometry.h>

#include <array>

Descriptor::Descriptor() {}
Descriptor::~Descriptor() {}

void Descriptor::createAndAllocateSimple(uint32_t images) {
	createSimpleLayout();
	createSimplePool(images);
}

void Descriptor::createSimpleLayout() {
	// Set up a binding
	VkDescriptorSetLayoutBinding binding = {};
	binding.binding = 0;
	binding.descriptorCount = 1;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	// Prepare to create the layout
	std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
		binding
	};

	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = static_cast<uint32_t>(bindings.size());
	info.pBindings = bindings.data();

	// Create the layout
	if (vkCreateDescriptorSetLayout(VulkanManager::getInstance()->getDevice()->logical, &info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("Unable to create Descriptor layout");
}

void Descriptor::createSimplePool(uint32_t images) {
	// Set the size of the pool we want
	VkDescriptorPoolSize size = {};
	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	size.descriptorCount = images;

	std::array<VkDescriptorPoolSize, 1> sizes = {
		size
	};

	// Prepare to create the pool
	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
	createInfo.pPoolSizes = sizes.data();
	createInfo.maxSets = images;

	// Create the pool
	if (vkCreateDescriptorPool(VulkanManager::getInstance()->getDevice()->logical, &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw std::runtime_error("Unable to create Descriptor pool");

	// Prepare to allocate the set
	std::vector<VkDescriptorSetLayout> layouts(images, layout);
	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.descriptorPool = pool;
	allocateInfo.descriptorSetCount = images;
	allocateInfo.pSetLayouts = layouts.data();

	// Allocate the Set
	if (vkAllocateDescriptorSets(VulkanManager::getInstance()->getDevice()->logical, &allocateInfo, &set) != VK_SUCCESS)
		throw std::runtime_error("Unable to allocate Descriptor set");
}

void Descriptor::populate(uint32_t images, VkBuffer uniforms, size_t bufferSize) {
	// Iterate images
	for (size_t i = 0; i < images; i++) {
		// Set up the uniform buffer
		VkDescriptorBufferInfo buffer = {};
		buffer.buffer = uniforms;
		buffer.offset = 0;
		buffer.range = bufferSize;

		// Prepare to write the buffer into the Descriptor set
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = set;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &buffer;
		write.pImageInfo = nullptr;
		write.pTexelBufferView = nullptr;

		std::array<VkWriteDescriptorSet, 1> writes = {
			write
		};

		// Write the buffer into the descriptor
		vkUpdateDescriptorSets(VulkanManager::getInstance()->getDevice()->logical, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}
}

void Descriptor::destroy() {
	vkDestroyDescriptorPool(VulkanManager::getInstance()->getDevice()->logical, pool, nullptr);
	vkDestroyDescriptorSetLayout(VulkanManager::getInstance()->getDevice()->logical, layout, nullptr);
}

VkDescriptorSetLayout createBindingWrapper(std::vector<VkDescriptorSetLayoutBinding> bindings) {
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = static_cast<uint32_t>(bindings.size());
	info.pBindings = bindings.data();

	VkDescriptorSetLayout layout;

	// Create the layout
	if (vkCreateDescriptorSetLayout(VulkanManager::getInstance()->getDevice()->logical, &info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("Unable to create Descriptor layout");

	return layout;
}
VkDescriptorPool createPoolWrapper(std::vector<VkDescriptorPoolSize> sizes, uint32_t images) {
	// Prepare to create the pool
	VkDescriptorPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	createInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
	createInfo.pPoolSizes = sizes.data();
	createInfo.maxSets = images;

	VkDescriptorPool pool;

	// Create the pool
	if (vkCreateDescriptorPool(VulkanManager::getInstance()->getDevice()->logical, &createInfo, nullptr, &pool) != VK_SUCCESS)
		throw std::runtime_error("Unable to create Descriptor pool");

	return pool;
}
