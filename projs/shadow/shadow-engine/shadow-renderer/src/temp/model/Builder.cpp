#include "temp/model/Builder.h"

namespace vlkxtemp {
    using namespace vlkx;
    using Geo::VertexAll;
    using VertexData = PerVertexBuffer::NoShareMeta;

    std::unique_ptr<SamplableImage> createTex(const ModelBuilder::TextureSource& source) {
        const auto usages = { ImageUsage::sampledFragment() };
        return std::make_unique<RefCountedTexture>(source, usages, ImageSampler::Config {});
    }

    void fillTextureMeta(const ModelBuilder::BindingPoints& binding, const ModelBuilder::TexturePerMesh& textures, const ModelBuilder::TexturePerMesh& sharedTextures, Descriptor::Meta* meta, Descriptor::ImageInfos* infos) {
        *meta = {Image::getSampleType(), VK_SHADER_STAGE_FRAGMENT_BIT, {} };
        auto& texBindings = meta->bindings;

        for (size_t idx = 0; idx < (size_t) ModelBuilder::TextureType::Count; idx++) {
            const auto type = (ModelBuilder::TextureType) idx;
            const size_t numTextures = textures[idx].size() + sharedTextures[idx].size();
            if (numTextures != 0) {
                const auto iter = binding.find(type);
                if (iter == binding.end())
                    throw std::runtime_error("Binding point of texture type " + std::to_string(idx) + " is not set");

                texBindings.push_back({ iter->second, static_cast<uint32_t>(numTextures) });

                auto& metaMap = (*infos)[iter->second];
                metaMap.reserve(numTextures);
                for (const auto& texture : textures[idx])
                    metaMap.push_back(texture->getInfoForSampling());
                for (const auto& texture : sharedTextures[idx])
                    metaMap.push_back(texture->getInfoForSampling());
            }
        }
    }

    std::vector<VkPushConstantRange> createRanges(const ModelBuilder::ModelPushConstant& constants) {
        std::vector<VkPushConstantRange> ranges;
        ranges.reserve(constants.constants.size());
        for (const auto& meta : constants.constants)
            ranges.push_back( { constants.stage, meta.offset, meta.constants->getSize() });
        return ranges;
    }

    VkVertexInputBindingDescription getBinding(uint32_t stride, bool instancing) {
        return VkVertexInputBindingDescription{ 0, stride,instancing ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX, };
    }

    void setVertexInput(const PerVertexBuffer& buffer, const std::vector<PerInstanceVertexBuffer*>& instanceBuffers, GraphicsPipelineBuilder* builder) {
        uint32_t start = 0;
        auto attributes = buffer.getAttrs(start);
        start += attributes.size();

        builder->addVertex(0, Geo::VertexAll::getBindingDesc(), std::move(attributes));

        for (size_t i = 0; i < instanceBuffers.size(); i++) {
            if (instanceBuffers[i] == nullptr)
                throw std::runtime_error("PerInstanceVertexBuffer not specified");
            auto instanceAttrs = instanceBuffers[i]->getAttrs(start);
            start += instanceAttrs.size();

            auto instanceBinding = getBinding(instanceBuffers[i]->getSize(), true);
            builder->addVertex(i + 1, std::move(instanceBinding), std::move(instanceAttrs));
        }
    }

    void ModelBuilder::SingleMeshModel::load(ModelBuilder* builder) const {
        const Wavefront obj(objFile, objIndexBase);
        VertexData vertices {
                {{
                    PerVertexBuffer::VertexDataMeta { obj.indices },
                    PerVertexBuffer::VertexDataMeta { obj.vertices }
                }}
        };

        builder->vertexBuffer = std::make_unique<StaticPerVertexBuffer>(std::move(vertices), Geo::VertexAll::getAttributeDesc());

        auto& meshTexs = builder->textures;
        meshTexs.push_back({});
        for (const auto& pair : textureSources) {
            const auto type = (size_t)pair.first;
            const auto& sources = pair.second;

            meshTexs.back()[type].reserve(sources.size());
            for (const auto& source : sources)
                meshTexs.back()[type].push_back({createTex(source)});
        }
    }

    void ModelBuilder::MultiMeshModel::load(ModelBuilder* builder) const {
        const ModelLoader loader(models, textures);
        std::vector<VertexData::PerMesh> data;
        data.reserve(loader.getMeshes().size());

        for (const auto& mesh : loader.getMeshes())
            data.push_back({ PerVertexBuffer::VertexDataMeta { mesh.indices }, PerVertexBuffer::VertexDataMeta { mesh.vertices } });

        builder->vertexBuffer = std::make_unique<StaticPerVertexBuffer>(VertexData { std::move(data)}, Geo::VertexAll::getAttributeDesc());

        const auto usages = { ImageUsage::sampledFragment() };
        auto& meshTexs = builder->textures;
        meshTexs.reserve(loader.getMeshes().size());
        for (const auto& mesh : loader.getMeshes()) {
            meshTexs.push_back({});
            for (const auto& tex : mesh.textures)
                meshTexs.back()[(size_t) tex.type].push_back(std::make_unique<RefCountedTexture>(tex.path, usages, ImageSampler::Config {}));
        }
    }

    ModelBuilder::ModelBuilder(std::string &&name, int frames, float aspect, const ModelResource &resource)
        : frames(frames), aspectRatio(aspect), uniformBufferMeta(frames), pipelineBuilder(std::make_unique<GraphicsPipelineBuilder>()) {
        pipelineBuilder->name(std::move(name));
        resource.load(this);
    }

    ModelBuilder& ModelBuilder::texture(TextureType type, const TextureSource &source) {
        sharedTextures[(size_t) type].push_back(createTex(source));
        return *this;
    }

    ModelBuilder& ModelBuilder::bindTextures(TextureType type, uint32_t point) {
        bindPoints[type] = point;
        return *this;
    }

    ModelBuilder& ModelBuilder::instanceBuffer(vlkx::PerInstanceVertexBuffer* buffer) {
        instanceBuffers.push_back(buffer);
        return *this;
    }

    ModelBuilder& ModelBuilder::uniform(VkShaderStageFlags stage, std::vector<vlkx::Descriptor::Meta::Binding> &&bindings) {
        uniformMeta.push_back({ UniformBuffer::getDescriptorType(), stage, std::move(bindings) });
        return *this;
    }

    ModelBuilder& ModelBuilder::uniformBuffer(uint32_t point, const vlkx::UniformBuffer &buffer) {
        for (size_t frame = 0; frame < frames; frame++) {
            const int chunk = buffer.isSingle() ? 0 : frame;
            uniformBufferMeta[frame][point].push_back( buffer.getDescriptorInfo(chunk) );
        }

        return *this;
    }

    ModelBuilder& ModelBuilder::pushStage(VkShaderStageFlags stage) {
        if (!pushConstants.has_value())
            pushConstants.emplace();

        pushConstants->stage = stage;
        return *this;
    }

    ModelBuilder& ModelBuilder::pushConstant(const vlkx::PushConstant* constant, uint32_t offset) {
        if (!pushConstants.has_value())
            pushConstants.emplace();

        pushConstants.value().constants.push_back( ModelPushConstant::Meta { constant, offset });
        return *this;
    }

    ModelBuilder& ModelBuilder::shader(VkShaderStageFlagBits stage, std::string &&file) {
        pipelineBuilder->shader(stage, std::move(file));
        return *this;
    }

    std::vector<ModelBuilder::Descriptors> ModelBuilder::createDescs() const {
        std::vector<Descriptors> descs(frames);
        auto infos = uniformMeta;
        infos.resize(infos.size() + 1);

        for (size_t frame = 0; frame < frames; frame++) {
            descs[frame].reserve(textures.size());

            for (const auto& tex : textures) {
                Descriptor::ImageInfos image;
                fillTextureMeta(bindPoints, tex, sharedTextures, &infos.back(), &image);

                descs[frame].push_back(std::make_unique<StaticDescriptor>(infos));
                descs[frame].back()->buffers(UniformBuffer::getDescriptorType(), uniformBufferMeta[frame]);
                descs[frame].back()->images(Image::getSampleType(), image);
            }
        }

        return descs;
    }

    std::unique_ptr<Model> ModelBuilder::build() {
        if (pushConstants.has_value())
            if (pushConstants->constants.empty())
                throw std::runtime_error("Model sets push constant present but no data.");

        auto descs = createDescs();
        pipelineBuilder->layout(
                { descs[0][0]->getLayout() },
                pushConstants.has_value() ? createRanges(pushConstants.value()) : std::vector<VkPushConstantRange> {}
        );

        setVertexInput(*vertexBuffer, instanceBuffers, pipelineBuilder.get());

        uniformMeta.clear();
        uniformBufferMeta.clear();

        return std::unique_ptr<Model> {
            new Model {
                aspectRatio, std::move(vertexBuffer), std::move(instanceBuffers), std::move(pushConstants),
                std::move(sharedTextures), std::move(textures), std::move(descs), std::move(pipelineBuilder)
            }
        };
    }

    void Model::update(bool opaque, const VkExtent2D &frame, VkSampleCountFlagBits samples,
                       const vlkx::RenderPass &pass, uint32_t subpass, bool flipY) {

        pipeline = (*pipelineBuilder)
                .depthTest(true, opaque)
                .multiSample(samples)
                .viewport({ { 0, 0, static_cast<float>(frame.width), static_cast<float>(frame.height), 0, 1 }, { { 0, 0 }, frame } })
                .renderPass(*pass, subpass)
                .colorBlend(std::vector<VkPipelineColorBlendAttachmentState>(pass.getAttachsInSubpass(subpass), vlkx::Pipeline::getAlphaBlendState(!opaque)))
                .build();
    }

    void Model::draw(const VkCommandBuffer &commands, int frame, uint32_t instances) const {
        pipeline->bind(commands);

        for (size_t i = 0; i < perInstanceBuffers.size(); i++)
            perInstanceBuffers[i]->bind(commands, i + 1, 0);

        if (pushConstants.has_value())
            for (const auto& meta : pushConstants->constants)
                meta.constants->upload(commands, pipeline->getLayout(), frame, meta.offset, pushConstants->stage);

        for (size_t mesh = 0; mesh < textures.size(); mesh++) {
            descriptors[frame][mesh]->bind(commands, pipeline->getLayout(), pipeline->getBind());
            vertexBuffer->draw(commands, 0, mesh, instances);
        }
    }
}