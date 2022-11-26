#include <vlkx/render/shader/GeoBuffers.h>
#include <vlkx/vulkan/Tools.h>
#include <vlkx/vulkan/VulkanManager.h>

GeoBuffers::GeoBuffers() {}
GeoBuffers::~GeoBuffers() {}

void GeoBuffers::createBuffers(Geo::MeshType type) {
	// Set vertex and indice data
	switch (type) {
		case Geo::MeshType::Triangle:
			Geo::Mesh::setTriData(vertices, indices);
			break;
		case Geo::MeshType::Quad:
			Geo::Mesh::setQuadData(vertices, indices);
			break;
		case Geo::MeshType::Cube:
			Geo::Mesh::setCubeData(vertices, indices);
			break;
		case Geo::MeshType::Sphere:
			Geo::Mesh::setSphereData(vertices, indices);
			break;
	}

	// Create buffers
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffer();
}

void GeoBuffers::createVertexBuffer() {
	// Gather the size of the buffer we need..
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// Create and bind a temporary buffer
	VkTools::ManagedBuffer staging = VkTools::createGPUBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->physical);

	// Get ready to write into our GPU buffer
	void* data;
	vmaMapMemory(VulkanManager::getInstance()->getAllocator(), staging.allocation, &data);

	// Copy vertex data into the buffer
	memcpy(data, vertices.data(), (size_t)bufferSize);

	// Unmap the buffer from stack space
	vmaUnmapMemory(VulkanManager::getInstance()->getAllocator(), staging.allocation);

	// Prepare an area in the GPU that we cannot see, that the vertex data can live permanently.
	vertexBuffer = VkTools::createGPUBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->physical, false);

	// Copy our data from the staging buffer into the new permanent buffer
	VkTools::copyGPUBuffer(staging.buffer, vertexBuffer.buffer, bufferSize, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->graphicsQueue, VulkanManager::getInstance()->getDevice()->queueData.graphics);

	// Cleanup the staging area.
	vmaDestroyBuffer(VulkanManager::getInstance()->getAllocator(), staging.buffer, staging.allocation);
}

void GeoBuffers::createIndexBuffer() {
	// Gather the size of the buffer we need..
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// Create and bind a temporary buffer
    VkTools::ManagedBuffer staging = VkTools::createGPUBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->physical);

	// Get ready to write into our GPU buffer
	void* data;
	vmaMapMemory(VulkanManager::getInstance()->getAllocator(), staging.allocation, &data);

	// Copy index data into the buffer
	memcpy(data, indices.data(), (size_t)bufferSize);

	// Unmap the buffer from stack space
	vmaUnmapMemory(VulkanManager::getInstance()->getAllocator(), staging.allocation);

	// Prepare an area in the GPU that we cannot see, that the vertex data can live permanently.
	indexBuffer = VkTools::createGPUBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->physical);

	// Copy our data from the staging buffer into the new permanent buffer
	VkTools::copyGPUBuffer(staging.buffer, indexBuffer.buffer, bufferSize, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->graphicsQueue, VulkanManager::getInstance()->getDevice()->queueData.graphics);

	// Cleanup the staging area.
	vmaDestroyBuffer(VulkanManager::getInstance()->getAllocator(), staging.buffer, staging.allocation);
}

void GeoBuffers::createUniformBuffer() {
	// Allocate a buffer of the right size - we don't need to copy uniforms.
	VkDeviceSize bufferSize = sizeof(Geo::UniformBufferObject);

	uniformBuffer = VkTools::createGPUBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VulkanManager::getInstance()->getDevice()->logical, VulkanManager::getInstance()->getDevice()->physical);
}

void GeoBuffers::destroy() {
	// Cleanup uniform buffers
	vmaDestroyBuffer(VulkanManager::getInstance()->getAllocator(), uniformBuffer.buffer, uniformBuffer.allocation);

	// Cleanup index buffers
	vmaDestroyBuffer(VulkanManager::getInstance()->getAllocator(), indexBuffer.buffer, indexBuffer.allocation);

	// Cleanup vertex buffers
	vmaDestroyBuffer(VulkanManager::getInstance()->getAllocator(), vertexBuffer.buffer, vertexBuffer.allocation);
}