#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <fstream>
#include "shadow/util/RefCounter.h"
#include "vlkx/vulkan/VulkanModule.h"

namespace vlkx {
    class Pipeline;

    // A simple wrapper for the shader module used by the pipeline.
    class ShaderModule {
    public:
        using CountedShader = shadowutil::RefCounter<ShaderModule>;
        using ReleasePool = CountedShader::AutoRelease;

        ShaderModule(const std::string& path);

        ShaderModule(const ShaderModule&) = delete;
        ShaderModule& operator=(const ShaderModule&) = delete;

        ~ShaderModule() {
            vkDestroyShaderModule(VulkanModule::getInstance()->getDevice()->logical, shader, nullptr);
        }

        const VkShaderModule& operator*() const { return shader; }

    private:
        VkShaderModule shader;
    };

    class PipelineBuilder {
    public:
        PipelineBuilder(const PipelineBuilder&) = delete;
        PipelineBuilder& operator=(const PipelineBuilder&) = delete;

        virtual ~PipelineBuilder() {
            vkDestroyPipelineCache(VulkanModule::getInstance()->getDevice()->logical, cache, nullptr);
        }

        virtual std::unique_ptr<Pipeline> build() const = 0;

    protected:
        PipelineBuilder(std::optional<int> maxCache);

        void setName(std::string&& n) { name = std::move(name); }

        void setLayout(std::vector<VkDescriptorSetLayout>&& descLayouts, std::vector<VkPushConstantRange>&& constants);

        const std::string& getName() const { return name; }

        bool hasLayout() const { return layoutInfo.has_value(); }
        const VkPipelineLayoutCreateInfo& getLayout() const { return layoutInfo.value(); }

    private:
        VkPipelineCache cache;
        std::string name;

        std::optional<VkPipelineLayoutCreateInfo> layoutInfo;
        std::vector<VkDescriptorSetLayout> descLayouts;
        std::vector<VkPushConstantRange> constants;
    };

    /**
     * Use when creating Graphics pipelines.
     * Internal state is preserved so that multiple pipelines can be created with one builder.
     * However, shaders are single-usage. Bind a new shader before claling build again.
     * See ShaderModule for more information, and how to change this.
     */
    class GraphicsPipelineBuilder : public PipelineBuilder {
    public:
        struct Viewport {
            VkViewport viewport;
            VkRect2D scissor;
        };

        explicit GraphicsPipelineBuilder(std::optional<int> maxCache = std::nullopt);

        GraphicsPipelineBuilder(const GraphicsPipelineBuilder&) = delete;
        GraphicsPipelineBuilder& operator=(const GraphicsPipelineBuilder&) = delete;

        #define fluent GraphicsPipelineBuilder&

        fluent name(std::string&& name);
        fluent depthTest(bool enable, bool write);
        fluent stencilTest(bool enable);
        fluent multiSample(VkSampleCountFlagBits samples);
        fluent topology(VkPrimitiveTopology topology);
        fluent stencilOp(const VkStencilOpState& state, VkStencilFaceFlags flags);

        fluent addVertex(uint32_t bindPoint, VkVertexInputBindingDescription&& desc, std::vector<VkVertexInputAttributeDescription>&& attrs);
        fluent layout(std::vector<VkDescriptorSetLayout>&& descLayouts, std::vector<VkPushConstantRange>&& constants);
        fluent viewport(const Viewport& port, bool flipY = true);
        fluent renderPass(const VkRenderPass& pass, uint32_t subpass);

        fluent colorBlend(std::vector<VkPipelineColorBlendAttachmentState>&& states);
        fluent shader(VkShaderStageFlagBits stage, std::string&& file);

        std::unique_ptr<Pipeline> build() const override;

    private:
        struct PassInfo {
            VkRenderPass pass;
            uint32_t subpass;
        };

        VkPipelineInputAssemblyStateCreateInfo assemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        std::vector<VkVertexInputBindingDescription> bindingDescs;
        std::vector<VkVertexInputAttributeDescription> attrDescs;

        std::optional<Viewport> viewportMeta;
        std::optional<PassInfo> passMeta;
        std::vector<VkPipelineColorBlendAttachmentState> blendStates;
        std::map<VkShaderStageFlagBits, std::string> shaders;
    };

    /**
     * Use when creating Compute Shader pipelines.
     * Internal state is preserved so that multiple pipelines can be created with one builder.
     * However, shaders are single-usage. Bind a new shader before claling build again.
     * See ShaderModule for more information, and how to change this.
     */
    class ComputePipelineBuilder : public PipelineBuilder {
    public:
        explicit ComputePipelineBuilder(std::optional<int> maxCache = std::nullopt) : PipelineBuilder(maxCache) {}

        ComputePipelineBuilder(const ComputePipelineBuilder&) = delete;
        ComputePipelineBuilder& operator=(const ComputePipelineBuilder&) = delete;

        #define fluent ComputePipelineBuilder&

        fluent name(std::string&& name);
        fluent layout(std::vector<VkDescriptorSetLayout>&& descLayouts, std::vector<VkPushConstantRange>&& pushConstants);
        fluent shader(std::string&& path);

        std::unique_ptr<Pipeline> build() const override;

    private:
        std::optional<std::string> shaderPath;
    };

    /**
     * Pipeline configures:
     *  - Shader Stages
     *  - Fixed Function stages
     *  - Vertex input bindings
     *  - Vertex attributes
     *  - Assembly
     *  - Tesselation
     *  - Viewport and Scissor
     *  - Rasterization
     *  - Multisampling
     *  - Depth testing
     *  - Stencil testing
     *  - Color blending
     *  - Dynamic states
     *  - Pipeline layout
     *      - Descriptor set layout
     *      - Push constant ranges
     *
     * Create a Pipeline with one of the builders above.
     */
    class Pipeline {
    public:
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        ~Pipeline();

        void bind(const VkCommandBuffer& buffer) const;

        const VkPipeline& operator*() const { return pipeline; }
        const VkPipelineLayout& getLayout() const { return layout; }
        VkPipelineBindPoint getBind() const { return bindPoint; }

        static VkPipelineColorBlendAttachmentState getAlphaBlendState(bool blending) {
            return {
                blending, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                VK_BLEND_OP_ADD, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };
        }

    private:
        friend std::unique_ptr<Pipeline> GraphicsPipelineBuilder::build() const;
        friend std::unique_ptr<Pipeline> ComputePipelineBuilder::build() const;

        Pipeline(std::string name, const VkPipeline& line, const VkPipelineLayout& lay, VkPipelineBindPoint bPoint)
         : name(std::move(name)), pipeline(line), layout(lay), bindPoint(bPoint) {}

        const std::string name;
        // The active Pipeline layout.
        const VkPipelineLayout layout;
        // The active Pipeline instance.
        const VkPipeline pipeline;
        // Whether this is a graphics or compute pipeline
        const VkPipelineBindPoint bindPoint;
    };
}
