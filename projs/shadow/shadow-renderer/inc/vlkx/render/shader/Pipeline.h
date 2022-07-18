#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>

class Pipeline {
public:
	Pipeline();
	~Pipeline();

	// The active Graphics Pipeline layout.
	VkPipelineLayout layout;
	// The active Graphics Pipeline instance.
	VkPipeline pipeline;

	// Create the layout and pipeline for a vertex renderer
	void create(VkExtent2D extent, VkDescriptorSetLayout set, VkRenderPass renderPass);

	void destroy();

private:

	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createPipelineLayout(VkDescriptorSetLayout set);
	
	// For rendering objects that use traditional vertex-based mesh geometry.
	// See Geo::Vertex for the uniform bindings.
	void createVertexPipeline(VkExtent2D extent, VkRenderPass renderPass);
};