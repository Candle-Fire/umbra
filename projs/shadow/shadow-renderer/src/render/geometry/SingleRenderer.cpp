#include <vlkx/render/geometry/SingleRenderer.h>
#include <vlkx/vulkan/VulkanManager.h>

#include <glm/gtc/quaternion.hpp>

void SingleRenderer::createSingleRenderer(Geo::MeshType type, glm::vec3 posIn, glm::vec3 scaleIn) {

	// Collect metadata about the swap chain
	uint32_t imageCount = VulkanManager::getInstance()->getSwapchain()->images.size();
	VkExtent2D imageExtent = VulkanManager::getInstance()->getSwapchain()->extent;

	// Create the buffers
	buffers.createBuffers(type);

	// Create the descriptor layout
	descriptor.createAndAllocateSimple(imageCount);
	descriptor.populate(imageCount, buffers.uniformBuffer.buffer, sizeof(Geo::UniformBufferObject));

	// Create the pipeline
	pipeline.create(imageExtent, descriptor.layout, VulkanManager::getInstance()->getRenderPass()->pass);

	// Set locals
	position = posIn;
	scale = scaleIn;
	rotation = glm::mat4_cast(glm::quat(0, 0, 0, 0));
}

void SingleRenderer::updateUniforms(Camera camera) {
	
	// Prepare data to go into the buffers
	Geo::UniformBufferObject ubo = {};
	
	glm::mat4 scaleMatrix = glm::mat4(1.0f);
	glm::mat4 rotationMatrix = getRotation();
	glm::mat4 translationMatrix = glm::mat4(1.0f);

	scaleMatrix = glm::scale(scaleMatrix, scale);
	translationMatrix = glm::translate(translationMatrix, position);

	ubo.model = translationMatrix * rotationMatrix * scaleMatrix;
	ubo.view = camera.getViewMatrix();
	ubo.proj = camera.getProjectionMatrix();

	ubo.proj[1][1] *= -1;

	// Copy the buffers into the GPU
	void* data;
	vmaMapMemory(VulkanManager::getInstance()->getAllocator(), buffers.uniformBuffer.allocation, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vmaUnmapMemory(VulkanManager::getInstance()->getAllocator(), buffers.uniformBuffer.allocation);
}

void SingleRenderer::draw() {
	// Fetch the buffer we're going to insert commands to
	VkCommandBuffer commands = VulkanManager::getInstance()->getCurrentCommandBuffer();

	// Bind our pipeline
	vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

	// Bind the vertex buffer
	VkBuffer vertexBuffers[] = {
		buffers.vertexBuffer.buffer
	};

	VkDeviceSize offsets[] = {
		0
	};

	vkCmdBindVertexBuffers(commands, 0, 1, vertexBuffers, offsets);

	// Bind the index buffer
	vkCmdBindIndexBuffer(commands, buffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	// Bind the uniform buffer
	vkCmdBindDescriptorSets(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.layout, 0, 1, &descriptor.set, 0, nullptr);

	// Draw the buffered geometry
	vkCmdDrawIndexed(commands, static_cast<uint32_t>(buffers.indices.size()), 1, 0, 0, 0);
}

void SingleRenderer::destroy() {
	pipeline.destroy();
	descriptor.destroy();
	buffers.destroy();
}