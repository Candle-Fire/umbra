#pragma once

#include "vlkx/render/shader/Pipeline.h"
#include "Loader.h"
#include "vlkx/render/render_pass/GenericRenderPass.h"
#include "vlkx/vulkan/abstraction/Descriptor.h"

namespace vlkxtemp {

    class Model;

    class ModelBuilder {
    public:

        using ShaderPool = vlkx::ShaderModule::ReleasePool;
        using TextureType = ModelLoader::TextureType;
        using TexturePerMesh = std::array<std::vector<std::unique_ptr<vlkx::SamplableImage>>, static_cast<int>(TextureType::Count)>;
        using BindingPoints = std::map<TextureType, uint32_t>;
        using TextureSource = vlkx::RefCountedTexture::ImageLocation;
        using TextureSources = std::map<TextureType, std::vector<TextureSource>>;

        class ModelResource {
        public:
            virtual ~ModelResource() = default;
            virtual void load(ModelBuilder* builder) const = 0;
        };

        class SingleMeshModel : public ModelResource {
        public:
            SingleMeshModel(std::string&& path, int indexBase, TextureSources&& sources)
                : objFile(std::move(path)), objIndexBase(indexBase), textureSources(std::move(sources)) {}

            void load(ModelBuilder* builder) const override;
        private:
            const std::string objFile;
            const int objIndexBase;
            const TextureSources textureSources;
        };

        class MultiMeshModel : public ModelResource {
        public:
            MultiMeshModel(std::string&& modelDir, std::string&& textureDir)
                : models(std::move(modelDir)), textures(std::move(textureDir)) {}

            void load(ModelBuilder* builder) const override;

        private:
            const std::string models;
            const std::string textures;
        };

        struct ModelPushConstant {
            struct Meta {
                const vlkx::PushConstant* constants;
                uint32_t offset;
            };

            VkShaderStageFlags stage;
            std::vector<Meta> constants;
        };

        using Descriptors = std::vector<std::unique_ptr<vlkx::StaticDescriptor>>;

        ModelBuilder(std::string&& name, int frames, float aspect, const ModelResource& resource);

        ModelBuilder(const ModelBuilder&) = delete;
        ModelBuilder& operator=(const ModelBuilder&) = delete;

        ModelBuilder& texture(TextureType type, const TextureSource& source);
        ModelBuilder& bindTextures(TextureType type, uint32_t point);
        ModelBuilder& instanceBuffer(vlkx::PerInstanceVertexBuffer* buffer);
        ModelBuilder& uniform(VkShaderStageFlags stage, std::vector<vlkx::Descriptor::Meta::Binding>&& bindings);
        ModelBuilder& uniformBuffer(uint32_t point, const vlkx::UniformBuffer& buffer);
        ModelBuilder& pushStage(VkShaderStageFlags stage);
        ModelBuilder& pushConstant(const vlkx::PushConstant* constant, uint32_t offset);
        ModelBuilder& shader(VkShaderStageFlagBits stage, std::string&& file);

        std::unique_ptr<Model> build();

    private:
        std::vector<Descriptors> createDescs() const;

        const int frames;
        const float aspectRatio;

        std::unique_ptr<vlkx::StaticPerVertexBuffer> vertexBuffer;
        std::vector<TexturePerMesh> textures;
        TexturePerMesh sharedTextures;
        BindingPoints bindPoints;

        std::vector<vlkx::PerInstanceVertexBuffer*> instanceBuffers;
        std::vector<vlkx::Descriptor::Meta> uniformMeta;
        std::vector<vlkx::Descriptor::BufferInfos> uniformBufferMeta;

        std::optional<ModelPushConstant> pushConstants;
        std::unique_ptr<vlkx::GraphicsPipelineBuilder> pipelineBuilder;
    };

    class Model {
    public:

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void update(bool opaque, const VkExtent2D& frame, VkSampleCountFlagBits samples, const vlkx::RenderPass& pass, uint32_t subpass, bool flipY = true);
        void draw(const VkCommandBuffer& commands, int frame, uint32_t instances) const;

    private:
        friend std::unique_ptr<Model> ModelBuilder::build();
        using Descriptors = ModelBuilder::Descriptors;
        using ModelPushConstant = ModelBuilder::ModelPushConstant;
        using TexturePerMesh = ModelBuilder::TexturePerMesh;

        Model(float aspectRatio,
              std::unique_ptr<vlkx::StaticPerVertexBuffer>&& vertexBuffer,
              std::vector<vlkx::PerInstanceVertexBuffer*>&& perInstanceBuffers,
              std::optional<ModelPushConstant>&& pushConstants,
              TexturePerMesh&& sharedTextures,
              std::vector<TexturePerMesh>&& textures,
              std::vector<Descriptors>&& descriptors,
              std::unique_ptr<vlkx::GraphicsPipelineBuilder>&& pipelineBuilder)
              : aspectRatio(aspectRatio), vertexBuffer(std::move(vertexBuffer)), perInstanceBuffers(std::move(perInstanceBuffers)),
                pushConstants(std::move(pushConstants)), sharedTextures(std::move(sharedTextures)), textures(std::move(textures)),
                descriptors(std::move(descriptors)), pipelineBuilder(std::move(pipelineBuilder)) {}

        const float aspectRatio;
        const std::unique_ptr<vlkx::StaticPerVertexBuffer> vertexBuffer;
        const std::vector<vlkx::PerInstanceVertexBuffer*> perInstanceBuffers;
        const std::optional<ModelPushConstant> pushConstants;
        const TexturePerMesh sharedTextures;
        const std::vector<TexturePerMesh> textures;
        const std::vector<Descriptors> descriptors;

        std::unique_ptr<vlkx::GraphicsPipelineBuilder> pipelineBuilder;
        std::unique_ptr<vlkx::Pipeline> pipeline;
    };
}