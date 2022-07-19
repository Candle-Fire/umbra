#include <vlkx/render/shader/Pipeline.h>
#include <vlkx/vulkan/VulkanManager.h>

#include <vlkx/render/Geometry.h>
#include <string>
#include <map>

Pipeline::Pipeline() {}
Pipeline::~Pipeline() {}

void Pipeline::create(VkExtent2D extent, VkDescriptorSetLayout set, VkRenderPass renderPass) {
	createPipelineLayout(set);
	createVertexPipeline(extent, renderPass);
}

void Pipeline::createPipelineLayout(VkDescriptorSetLayout set) {
	// Prepare to create the pipeline layout
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO; 
	info.setLayoutCount = 1;
	info.pSetLayouts = &set;
	
	// Create the layout
	if (vkCreatePipelineLayout(VulkanManager::getInstance()->getDevice()->logical, &info, nullptr, &layout) != VK_SUCCESS)
		throw std::runtime_error("Unable to create Pipeline Layout.");
}

std::vector<char> Pipeline::readFile(const std::string& filename) {
	// Prepare the stream for reading the file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error(std::string("Unable to open shader file: ").append(filename));

	size_t fileSize = (size_t)file.tellg();

	// Read the file into a vector
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

VkShaderModule Pipeline::createShaderModule(const std::vector<char>& code) {
	// Prepare to create the shader module
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = code.size();
	info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	// Create the module
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VulkanManager::getInstance()->getDevice()->logical, &info, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Unable to create Shader module from SPIR-V binary");

	return shaderModule;
}

void Pipeline::createVertexPipeline(VkExtent2D extent, VkRenderPass renderPass) {

	// ------- Stage: Shader ------- //

	// Read the vertex shader
	auto vertexShaderCode = readFile("vlkx-resources/shader/SPIRV/basic.vert.spv");
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);

	// Prepare to create the Shader Stage
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderCreateInfo.module = vertexShaderModule;
	vertexShaderCreateInfo.pName = "main";


	// Read the fragment shader
	auto fragmentShaderCode = readFile("vlkx-resources/shader/SPIRV/basic.frag.spv");
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

	// Prepare to create the Shader Stage
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = {};
	fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderCreateInfo.module = fragmentShaderModule;
	fragmentShaderCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertexShaderCreateInfo,
		fragmentShaderCreateInfo
	};

	// ------- Stage: Shader Input ------- //

	auto bindingDescription = Geo::Vertex::getBindingDesc();
	auto attributeDescription = Geo::Vertex::getAttributeDesc();

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescription.data();

	// ------- Stage: Input Assembly ------- //

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	// ------- Stage: Rasterization ------- //

	VkPipelineRasterizationStateCreateInfo rasterCreateInfo = {};
	rasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterCreateInfo.depthClampEnable = VK_FALSE;
	rasterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterCreateInfo.lineWidth = 1.0f;
	rasterCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterCreateInfo.depthBiasEnable = VK_FALSE;
	rasterCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterCreateInfo.depthBiasClamp = 0.0f;
	rasterCreateInfo.depthBiasSlopeFactor = 0.0f;

	// ------- Stage: MultiSample ------- //

	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// ------- Stage: Color Blending ------- //

	VkPipelineColorBlendAttachmentState colorBlendAttach = {};
	colorBlendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttach.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
	colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendCreateInfo.attachmentCount = 1;
	colorBlendCreateInfo.pAttachments = &colorBlendAttach;

	// ------- Stage: Viewport ------- //

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo = {};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.pViewports = &viewport;
	viewportCreateInfo.scissorCount = 1;
	viewportCreateInfo.pScissors = &scissor;

	// ------- Create the Pipeline ------- //

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineInfo.pRasterizationState = &rasterCreateInfo;
	pipelineInfo.pMultisampleState = &multisampleCreateInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlendCreateInfo;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.pViewportState = &viewportCreateInfo;

	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(VulkanManager::getInstance()->getDevice()->logical, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw std::runtime_error("Unable to create a Vertex rendering Pipeline.");


	// ------- Cleanup temporary allocations ------- //

	vkDestroyShaderModule(VulkanManager::getInstance()->getDevice()->logical, vertexShaderModule, nullptr);
	vkDestroyShaderModule(VulkanManager::getInstance()->getDevice()->logical, fragmentShaderModule, nullptr);
}


void Pipeline::destroy() {
	vkDestroyPipeline(VulkanManager::getInstance()->getDevice()->logical, pipeline, nullptr);
	vkDestroyPipelineLayout(VulkanManager::getInstance()->getDevice()->logical, layout, nullptr);
}