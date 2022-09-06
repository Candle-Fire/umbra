#include "vlkx/vulkan/abstraction/Buffer.h"
#include "vlkx/vulkan/Tools.h"
#include "vlkx/vulkan/VulkanModule.h"
#include <memory>

namespace vlkx {

    void executeBulkCopy(VkTools::ManagedBuffer buffer, const std::vector<vlkx::Buffer::CopyMeta>& meta) {
        void* dst;
        vmaMapMemory(VulkanModule::getInstance()->getAllocator(), buffer.allocation, &dst);
        // GPU memory accessible through dst pointer

        for (const auto& info : meta) {
            memcpy(static_cast<char*>(dst) + info.start, info.data, info.length);
        }

        // Unmap GPU memory
        vmaUnmapMemory(VulkanModule::getInstance()->getAllocator(), buffer.allocation);
    }

    StagingBuffer::StagingBuffer(const vlkx::Buffer::BulkCopyMeta &copyMeta) : dataSize(copyMeta.length) {
        setBuffer(VkTools::createGPUBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VulkanModule::getInstance()->getDevice()->logical, VulkanModule::getInstance()->getDevice()->physical));

        executeBulkCopy(get(), copyMeta.metas);
    }

    void StagingBuffer::copy(const VkBuffer &target) const {
        VkTools::copyGPUBuffer(get().buffer, target, dataSize, VulkanModule::getInstance()->getDevice());
    }

    std::vector<VkVertexInputAttributeDescription> VertexBuffer::getAttrs(uint32_t start) const {
        std::vector<VkVertexInputAttributeDescription> descs;
        descs.reserve(attributes.size());

        for (const auto& attr : attributes) {
            descs.push_back(VkVertexInputAttributeDescription {
                start++, 0, attr.format, attr.offset
            });
        }

        return descs;
    }

    void VertexBuffer::draw(const VkCommandBuffer &commands, uint32_t verts, uint32_t instances) {
        vkCmdDraw(commands, verts, instances, 0, 0);
    }

    void VertexBuffer::create(VkDeviceSize totalSize, bool dynamic, bool indexes) {
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VkMemoryPropertyFlags props;

        if (dynamic) {
            props = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        } else {
            usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }

        if (indexes)
            usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        setBuffer(VkTools::createGPUBuffer(totalSize, usage, props, VulkanModule::getInstance()->getDevice()->logical, VulkanModule::getInstance()->getDevice()->physical, dynamic));
    }

    DynamicBuffer::DynamicBuffer(size_t size, bool hasIndices, vlkx::VertexBuffer *buffer) : hasIndices(hasIndices), vertexBuffer(buffer) {
        resize(size);
    }

    void DynamicBuffer::resize(size_t pSize) {
        if (pSize <= bufferSize()) return;

        if (pSize > 0) {
            // todo: release the buffer & device memory
        }

        size = pSize;
        vertexBuffer->create(size, true, hasIndices);
    }

    Buffer::BulkCopyMeta PerVertexBuffer::NoIndexBufferMeta::prepareCopy(vlkx::PerVertexBuffer *buffer) const {
        auto& meshInfos = buffer->meshDataInfo.emplace<MeshDataNoIndex>().info;
        meshInfos.reserve(perMeshVertices.size());

        std::vector<Buffer::CopyMeta> copyMetas;
        copyMetas.reserve(perMeshVertices.size());

        VkDeviceSize offset = 0;
        for (const auto& verts : perMeshVertices) {
            meshInfos.push_back(MeshDataNoIndex::Info { static_cast<uint32_t>(verts.unitsPerMesh), offset });
            copyMetas.push_back(Buffer::CopyMeta { verts.data, verts.sizePerMesh, offset });
            offset += verts.sizePerMesh;
        }

        return Buffer::BulkCopyMeta { offset, std::move(copyMetas) };
    }

    Buffer::BulkCopyMeta PerVertexBuffer::SharedIndexMeta::prepareCopy(vlkx::PerVertexBuffer *buffer) const {
        auto& meshInfos = buffer->meshDataInfo.emplace<MeshDataIndex>().info;
        meshInfos.reserve(meshes);

        VkDeviceSize offset = sharedIndices.sizePerMesh;
        for (int i = 0; i < meshes; ++i) {
            meshInfos.push_back(MeshDataIndex::Info { static_cast<uint32_t>(sharedIndices.unitsPerMesh), 0, offset });
            offset += perMeshVertex.sizePerMesh;
        }

        return Buffer::BulkCopyMeta { offset, { { sharedIndices.data, sharedIndices.sizePerMesh, 0 }, { perMeshVertex.data, perMeshVertex.sizePerMesh * meshes, sharedIndices.sizePerMesh } } };
    }


    Buffer::BulkCopyMeta PerVertexBuffer::NoShareMeta::prepareCopy(vlkx::PerVertexBuffer *buffer) const {
        auto& meshInfos = buffer->meshDataInfo.emplace<MeshDataIndex>().info;
        meshInfos.reserve(perMeshMeta.size());

        std::vector<Buffer::CopyMeta> copyMetas;
        copyMetas.reserve(perMeshMeta.size() * 2);

        VkDeviceSize offset = 0;
        for (const auto& meta : perMeshMeta) {
            const size_t indicesSize = meta.indices.sizePerMesh;
            const size_t verticesSize = meta.vertices.sizePerMesh;
            const VkDeviceSize verticesOffset = offset + indicesSize;

            meshInfos.push_back(MeshDataIndex::Info { static_cast<uint32_t>(meta.indices.unitsPerMesh), offset, verticesOffset });
            copyMetas.push_back(Buffer::CopyMeta { meta.indices.data, indicesSize, offset });
            copyMetas.push_back(Buffer::CopyMeta { meta.vertices.data, verticesSize, verticesOffset });

            offset += indicesSize + verticesSize;
        }

        return Buffer::BulkCopyMeta { offset, std::move(copyMetas) };
    }

    void PerVertexBuffer::draw(const VkCommandBuffer &commands, uint32_t bind, int index, uint32_t instances) const {
        if (const auto* meshNoIndex = std::get_if<MeshDataNoIndex>(&meshDataInfo); meshNoIndex != nullptr) {
            const auto& meshInfo = meshNoIndex->info[index];
            vkCmdBindVertexBuffers(commands, bind, 1, &getBuffer(), &meshInfo.vertexStart);
            vkCmdDraw(commands, meshInfo.vertexCount, instances, 0, 0);
        } else if (const auto* meshIndex = std::get_if<MeshDataIndex>(&meshDataInfo); meshIndex != nullptr) {
            const auto& meshInfo = meshIndex->info[index];
            vkCmdBindIndexBuffer(commands, getBuffer(), meshInfo.indexStart, VK_INDEX_TYPE_UINT32);
            vkCmdBindVertexBuffers(commands, bind, 1, &getBuffer(), &meshInfo.vertexStart);
            vkCmdDrawIndexed(commands, meshInfo.indexCount, instances, 0, 0, 0);
        }
    }

    StaticPerVertexBuffer::StaticPerVertexBuffer(const vlkx::PerVertexBuffer::BufferDataMeta &info,
                                                 std::vector<VkVertexInputAttributeDescription> &&attrs) : PerVertexBuffer(std::move(attrs)) {
        const BulkCopyMeta copy = info.prepareCopy(this);
        create(copy.length, false, info.hasIndices());
        const StagingBuffer staging(copy);
        staging.copy(getBuffer());
    }

    void DynamicPerVertexBuffer::copyToDevice(const vlkx::PerVertexBuffer::BufferDataMeta &meta) {
        const BulkCopyMeta copy = meta.prepareCopy(this);
        resize(copy.length);
        executeBulkCopy(get(), copy.metas);
    }

    void PerInstanceVertexBuffer::bind(const VkCommandBuffer &commands, uint32_t bindPoint, int offset) const {
        const VkDeviceSize size = sizePerInstance * offset;
        vkCmdBindVertexBuffers(commands, bindPoint, 1, &getBuffer(), &size);
    }

    StaticPerInstanceBuffer::StaticPerInstanceBuffer(uint32_t size, const void *data, uint32_t instances,
                                                     std::vector<VkVertexInputAttributeDescription> &&attrs) : PerInstanceVertexBuffer(size, std::move(attrs)) {
        const uint32_t totalSize = size * instances;
        create(totalSize, false, false);

        const BulkCopyMeta copy { totalSize, { {data, totalSize, 0} } };
        const StagingBuffer staging(copy);
        staging.copy(getBuffer());
    }

    void DynamicPerInstanceBuffer::copyToDevice(const void *data, uint32_t instances) {
        const uint32_t totalSize = getSize() * instances;
        const BulkCopyMeta copy { totalSize, { { data, totalSize, 0 } } };
        resize(totalSize);
        executeBulkCopy(get(), copy.metas);
    }

    UniformBuffer::UniformBuffer(size_t chunkSize, int chunks) : DataBuffer(), chunkSize(chunkSize), numChunks(chunks) {
        const VkDeviceSize alignment = VulkanModule::getInstance()->getDevice()->limits.minUniformBufferOffsetAlignment;
        chunkLength = (chunkSize + alignment - 1) / alignment * alignment;

        data = new char[chunkSize * numChunks];
        setBuffer(VkTools::createGPUBuffer(chunkLength * numChunks, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VulkanModule::getInstance()->getDevice()->logical, VulkanModule::getInstance()->getDevice()->physical, true));
    }

    void UniformBuffer::upload(int index) const {
        checkIndex(index);
        const VkDeviceSize srcOffset = chunkSize * index;
        const VkDeviceSize dstOffset = chunkLength * index;

        // TODO: dstoffset?
        executeBulkCopy(get(), { { data + srcOffset, chunkSize, 0 } } );
    }

    void UniformBuffer::upload(int index, VkDeviceSize dataSize, VkDeviceSize start) const {
        checkIndex(index);
        const VkDeviceSize srcOffset = chunkSize * index + start;
        const VkDeviceSize dstOffset = chunkLength * index + start;

        // TODO: dstoffset?
        executeBulkCopy(get(), { { data + srcOffset, dataSize, 0 } } );
    }

    VkDescriptorBufferInfo UniformBuffer::getDescriptorInfo(int index) const {
        checkIndex(index);
        return VkDescriptorBufferInfo { getBuffer(), chunkLength * index, chunkSize};
    }

    void UniformBuffer::checkIndex(int index) const {
        if (index > numChunks)
            throw std::runtime_error("Attempting to access uniform chunk " + std::to_string(index) + " out of range.");

    }

    PushConstant::PushConstant(size_t size, int numFrames) : sizePerFrame(static_cast<uint32_t>(size)), numFrames(numFrames) {
        if (size > 128)
            throw std::runtime_error("Attempting to push constant of size " + std::to_string(size) + ", max ix 128.");

        data = new char[size * numFrames];
    }

    void PushConstant::upload(const VkCommandBuffer &commands, const VkPipelineLayout &pipelineLayout, int frame,
                              uint32_t offset, VkShaderStageFlags stage) const {
        checkIndex(frame);
        void* data = getData<void>(frame);
        vkCmdPushConstants(commands, pipelineLayout, stage, offset, sizePerFrame, data);
    }

    void PushConstant::checkIndex(int index) const {
        if (index > numFrames)
            throw std::runtime_error("Attempting to access push constant for frame " + std::to_string(index) + " out of range.");

    }
}
