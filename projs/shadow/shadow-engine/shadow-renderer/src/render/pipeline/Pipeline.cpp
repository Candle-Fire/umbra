#include "vlkx/render/shader/Pipeline.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "shadow/util/File.h"
#include <numeric>

namespace vlkx {

    struct ShaderStage {
        VkShaderStageFlagBits stage;
        ShaderModule::CountedShader module;
    };

    VkPipelineViewportStateCreateInfo createViewport(const GraphicsPipelineBuilder::Viewport& port) {
        return {
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            nullptr, 0, 1, &port.viewport, 1, &port.scissor
        };
    }

    VkPipelineColorBlendStateCreateInfo createBlend(const std::vector<VkPipelineColorBlendAttachmentState>& states) {
        return {
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            nullptr, 0, VK_FALSE, VK_LOGIC_OP_CLEAR, static_cast<uint32_t>(states.size()), states.data(),
            { 0, 0, 0, 0 }
        };
    }

    VkPipelineVertexInputStateCreateInfo createVertexInput(const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                                                           const std::vector<VkVertexInputAttributeDescription>& attrDescs) {
        return {
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            nullptr, 0,
            static_cast<uint32_t>(bindingDescs.size()), bindingDescs.data(),
            static_cast<uint32_t>(attrDescs.size()), attrDescs.data()
        };
    }

    std::vector<ShaderStage> createShader(const std::map<VkShaderStageFlagBits, std::string>& shaderMap) {
        std::vector<ShaderStage> stages;
        stages.reserve(shaderMap.size());
        for (const auto& pair : shaderMap) {
            stages.push_back({ pair.first, ShaderModule::CountedShader::get(pair.second, pair.second) });
        }

        return stages;
    }

    std::vector<VkPipelineShaderStageCreateInfo> createShaderStage(const std::vector<ShaderStage>& stages) {
        static constexpr char entryPoint[] = "main";
        std::vector<VkPipelineShaderStageCreateInfo> infos;
        infos.reserve(stages.size());
        for (const auto& stage : stages) {
            infos.push_back({
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                nullptr, 0, stage.stage, **stage.module, entryPoint, nullptr
            });
        }

        return infos;
    }

    ShaderModule::ShaderModule(const std::string &path) {
        const shadowutil::FileData* file = shadowutil::loadFile(path);
        const VkShaderModuleCreateInfo module {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            nullptr, 0, file->size, reinterpret_cast<const uint32_t*>(file->data.data())
        };

        if (vkCreateShaderModule(VulkanModule::getInstance()->getDevice()->logical, &module, nullptr, &shader) != VK_SUCCESS)
            throw std::runtime_error("Unable to create shader module");
    }

    PipelineBuilder::PipelineBuilder(std::optional<int> maxCache) {
        const VkPipelineCacheCreateInfo info {
            VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            nullptr, 0, static_cast<size_t>(maxCache.value_or(0)), nullptr
        };

        if (vkCreatePipelineCache(VulkanModule::getInstance()->getDevice()->logical, &info, nullptr, &cache) != VK_SUCCESS)
            throw std::runtime_error("Unable to create pipeline cache");
    }

    void PipelineBuilder::setLayout(std::vector<VkDescriptorSetLayout>&& descs,
                                    std::vector<VkPushConstantRange>&& pushConstants) {
        std::vector<int> pushSizes (pushConstants.size());
        for (size_t i = 0; i < pushConstants.size(); i++)
            pushSizes[i] = pushConstants[i].size;

        const auto totalSize = std::accumulate(pushSizes.begin(), pushSizes.end(), 0);
        if (totalSize > 128)
            throw std::runtime_error("Trying to set push constants of total size " + std::to_string(totalSize) + " into pipeline " + name);

        descLayouts = std::move(descs);
        constants = std::move(pushConstants);
        layoutInfo.emplace(VkPipelineLayoutCreateInfo {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            nullptr, 0, static_cast<uint32_t>(descLayouts.size()), descLayouts.data(),
            static_cast<uint32_t>(constants.size()), constants.data()
        });
    }

    GraphicsPipelineBuilder::GraphicsPipelineBuilder(std::optional<int> maxCache) : PipelineBuilder(maxCache) {
        assemblyInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
        };

        rasterizationInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
                VK_FALSE, 0, 0, 0, 1
        };

        multisampleInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                nullptr, 0, VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 0, nullptr, VK_FALSE, VK_FALSE
        };

        depthStencilInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                nullptr, 0, VK_FALSE, VK_FALSE,
                VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE, VK_FALSE,
                {}, {}, 0, 1
        };

        dynamicStateInfo = {
                VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                nullptr, 0, 0, nullptr
        };
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::name(std::string &&name) {
        setName(std::move(name));
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::depthTest(bool enable, bool write) {
        depthStencilInfo.depthTestEnable = enable;
        depthStencilInfo.depthWriteEnable = write;
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::stencilTest(bool enable) {
        depthStencilInfo.stencilTestEnable = enable;
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::multiSample(VkSampleCountFlagBits samples) {
        multisampleInfo.rasterizationSamples = samples;
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::topology(VkPrimitiveTopology topology) {
        assemblyInfo.topology = topology;
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::stencilOp(const VkStencilOpState &state,
                                                                VkStencilFaceFlags flags) {
        if (flags & VK_STENCIL_FACE_FRONT_BIT)
            depthStencilInfo.front = state;
        if (flags & VK_STENCIL_FACE_BACK_BIT)
            depthStencilInfo.back = state;
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::addVertex(uint32_t bindPoint,
                                                                VkVertexInputBindingDescription &&desc,
                                                                std::vector<VkVertexInputAttributeDescription> &&attrs) {
        desc.binding = bindPoint;
        for (auto& attr : attrs)
            attr.binding = bindPoint;
        bindingDescs.push_back(desc);

        attrDescs.reserve(attrDescs.size() + attrs.size());
        std::move(attrs.begin(), attrs.end(), std::back_inserter(attrDescs));
        attrs.clear();
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::layout(std::vector<VkDescriptorSetLayout> &&descLayouts,
                                                             std::vector<VkPushConstantRange> &&constants) {
        setLayout(std::move(descLayouts), std::move(constants));
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::viewport(const vlkx::GraphicsPipelineBuilder::Viewport &port,
                                                               bool flipY) {
        viewportMeta.emplace(port);

        if (flipY) {
            VkViewport& view = viewportMeta.value().viewport;
            view.y += view.height;
            view.height *= -1;
        }

        rasterizationInfo.frontFace = flipY ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::renderPass(const VkRenderPass &pass, uint32_t subpass) {
        passMeta.emplace(PassInfo { pass, subpass });
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::colorBlend(
            std::vector<VkPipelineColorBlendAttachmentState> &&states) {
        blendStates = std::move(states);
        return *this;
    }

    GraphicsPipelineBuilder& GraphicsPipelineBuilder::shader(VkShaderStageFlagBits stage, std::string &&file) {
        shaders[stage] = std::move(file);
        return *this;
    }

    std::unique_ptr<Pipeline> GraphicsPipelineBuilder::build() const {
        if (!hasLayout())
            throw std::runtime_error("Pipeline " + getName() + " has no layout set");
        if (!viewportMeta.has_value())
            throw std::runtime_error("Pipeline " + getName() + " has no viewport set");
        if (!passMeta.has_value())
            throw std::runtime_error("Pipeline " + getName() + " has no render pass set");
        if (blendStates.empty())
            throw std::runtime_error("Pipeline " + getName() + " has no color blend states.");
        if (shaders.empty())
            throw std::runtime_error("Pipeline " + getName() + " has no shaders bound");

        const auto viewportState = createViewport(viewportMeta.value());
        const auto blendState = createBlend(blendStates);
        const auto vertexState = createVertexInput(bindingDescs, attrDescs);

        const auto shaderStages = createShader(shaders);
        const auto shaderStageInfo = createShaderStage(shaderStages);
        VkPipelineLayout pipelineLayout;
        if (vkCreatePipelineLayout(VulkanModule::getInstance()->getDevice()->logical, &getLayout(), nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create layout for pipeline " + getName());

        const VkGraphicsPipelineCreateInfo createInfo {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0,
            static_cast<uint32_t>(shaderStageInfo.size()), shaderStageInfo.data(),
            &vertexState, &assemblyInfo, nullptr, &viewportState,
            &rasterizationInfo, &multisampleInfo, &depthStencilInfo, &blendState, &dynamicStateInfo,
            pipelineLayout, passMeta->pass, passMeta->subpass, VK_NULL_HANDLE, 0
        };

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(VulkanModule::getInstance()->getDevice()->logical, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline " + getName());

        return std::unique_ptr<Pipeline> {
            new Pipeline(getName(), pipeline, pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS)
        };
    }

    ComputePipelineBuilder& ComputePipelineBuilder::name(std::string &&name) {
        setName(std::move(name));
        return *this;
    }

    ComputePipelineBuilder& ComputePipelineBuilder::layout(std::vector<VkDescriptorSetLayout> &&descLayouts,
                                                              std::vector<VkPushConstantRange> &&pushConstants) {
        setLayout(std::move(descLayouts), std::move(pushConstants));
        return *this;
    }

    ComputePipelineBuilder& ComputePipelineBuilder::shader(std::string &&path) {
        shaderPath.emplace(std::move(path));
        return *this;
    }

    std::unique_ptr<Pipeline> ComputePipelineBuilder::build() const {
        if (!hasLayout())
            throw std::runtime_error("Pipeline " + getName() + " has no layout set");
        if (!shaderPath.has_value())
            throw std::runtime_error("Pipeline " + getName() + " has no shader set");

        const auto shaderStages = createShader({{VK_SHADER_STAGE_COMPUTE_BIT, shaderPath.value()}});
        const auto shaderStageInfo = createShaderStage(shaderStages);
        if (shaderStageInfo.size() != 1)
            throw std::runtime_error("Compute pipeline " + getName() + " must have exactly one shader bound");

        VkPipelineLayout layout;
        if (vkCreatePipelineLayout(VulkanModule::getInstance()->getDevice()->logical, &getLayout(), nullptr,
                                   &layout) != VK_SUCCESS)
            throw std::runtime_error("Unable to create layout for compute pipeline " + getName());

        const VkComputePipelineCreateInfo createInfo{
                VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                nullptr, 0,
                shaderStageInfo[0], layout, VK_NULL_HANDLE, 0
        };

        VkPipeline pipeline;
        if (vkCreateComputePipelines(VulkanModule::getInstance()->getDevice()->logical, VK_NULL_HANDLE, 1, &createInfo,
                                     nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("Unable to create compute pipeline " + getName());

        return std::unique_ptr<Pipeline>{
                new Pipeline{getName(), pipeline, layout, VK_PIPELINE_BIND_POINT_COMPUTE}
        };
    }

    void Pipeline::bind(const VkCommandBuffer &buffer) const {
        vkCmdBindPipeline(buffer, bindPoint, pipeline);
    }

    Pipeline::~Pipeline() {
        vkDestroyPipeline(VulkanModule::getInstance()->getDevice()->logical, pipeline, nullptr);
        vkDestroyPipelineLayout(VulkanModule::getInstance()->getDevice()->logical, layout, nullptr);
    }
}