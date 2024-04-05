#pragma once

#include <vulkan/vulkan.h>
#include "vlkx/vulkan/VulkanDevice.h"
#include <variant>
#include <vector>
#include "vlkx/vulkan/Tools.h"

namespace vlkx {
    // Root buffer class.
    // Used to store & move data between the CPU and GPU.
    // Basically; hotspot!
    // Utilities and subclasses exist to optimise and speed up transfer and management of data in bulk.
    class Buffer {
    public:
        // Metadata of CPU->GPU data copying.
        struct CopyMeta {
            const void* data;    // The start of data in RAM
            VkDeviceSize length; // The amount of data to move
            VkDeviceSize start;  // The start (destination) in GPU memory
        };

        // Metadata of bulk CPU->GPU data copying.
        struct BulkCopyMeta {
            VkDeviceSize length;  // The total data size of all transfers.
            std::vector<CopyMeta> metas;
        };

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer();

        virtual ~Buffer() {
        }

    protected:
    };

    /*******************************************/
    /**                                        */
    /**         START OF DATA BUFFERS          */
    /**                                        */
    /*******************************************/

    // A buffer that stores data on GPU.
    // Usage of the data is determined by child classes.
    class DataBuffer : public Buffer {
    public:
        DataBuffer(const DataBuffer&) = delete;
        DataBuffer& operator=(const DataBuffer&) = delete;

        DataBuffer() = default;

        ~DataBuffer() override {
            vmaDestroyBuffer(VkTools::allocator, managed.buffer, managed.allocation);
        }

    protected:
        using Buffer::Buffer;

        void setBuffer(const VkTools::ManagedBuffer& buffer) { managed = buffer; }
        VkTools::ManagedBuffer get() const { return managed; }
        const VkBuffer& getBuffer() const { return managed.buffer; }

    private:
        VkTools::ManagedBuffer managed;
    };

    // A buffer visible to both GPU and CPU.
    // Useful for uploading data to GPU for format conversions.
    class StagingBuffer : public DataBuffer {
    public:
        StagingBuffer(const BulkCopyMeta& copyMeta);

        StagingBuffer(const StagingBuffer&) = delete;
        StagingBuffer& operator=(const StagingBuffer&) = delete;

        void copy(const VkBuffer& target) const;

    private:
        const VkDeviceSize dataSize;
    };

    // Root class of vertex buffers.
    // Provides utilities for subclasses.
    class VertexBuffer : public DataBuffer {
    public:
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        // Get attributes of vertexes in the buffer
        // Location will start from "start"
        // Binding will not be set
        std::vector<VkVertexInputAttributeDescription> getAttrs(uint32_t start) const;

        // Draw these vertexes without a buffer per vertex.
        static void draw(const VkCommandBuffer& buffer, uint32_t verts, uint32_t instances);

    protected:
        friend class DynamicBuffer;

        explicit VertexBuffer(std::vector<VkVertexInputAttributeDescription>&& attrs) : DataBuffer(), attributes(attrs) {}

        // Initialize device memory and the managed buffer.
        // indices and vertexes are put in the same buffer.
        // if dynamic, the buffer will be host visible, this allows dynamic text.
        // otherwise, the buffer is device local.
        void create(VkDeviceSize totalSize, bool dynamic, bool indexes);

        const std::vector<VkVertexInputAttributeDescription> attributes;
    };

    /*******************************************/
    /**                                        */
    /**           END OF DATA BUFFERS          */
    /**                                        */
    /*******************************************/

    // A simple plugin to allow the vertex buffer to be widened when reserved with a larger size.
    class DynamicBuffer {
    public:
        DynamicBuffer(const DynamicBuffer&) = delete;
        DynamicBuffer& operator=(const DynamicBuffer&) = delete;

        ~DynamicBuffer() = default;

    protected:

        DynamicBuffer(size_t size, bool hasIndices, VertexBuffer* buffer);

        // Reallocate the vertex buffer if the given pSize is larger than the available space
        void resize(size_t pSize);

        VkDeviceSize bufferSize() const { return size; }

    private:

        const bool hasIndices;
        VertexBuffer* vertexBuffer;
        VkDeviceSize size = 0;
    };

    /*******************************************/
    /**                                        */
    /**      START OF PER-VERTEX BUFFERS       */
    /**                                        */
    /*******************************************/

    // Root class of buffers that store per-vertex data.
    // eg. shader data
    class PerVertexBuffer : public VertexBuffer {
    public:
        // Interprets the layout of data in containers (vector, etc)
        struct VertexDataMeta {
            template <typename C>
            VertexDataMeta(const C& cont, int unitsPerMesh) : data(cont.data()), unitsPerMesh(unitsPerMesh), sizePerMesh(sizeof(cont[0]) * unitsPerMesh) {}

            template <typename C>
            VertexDataMeta(const C& cont) : VertexDataMeta(cont, static_cast<int>(cont.size())) {}

            const void* data;
            int unitsPerMesh;
            size_t sizePerMesh;
        };

        // Interface for buffer data.
        class BufferDataMeta {
        public:
            virtual ~BufferDataMeta() = default;
            // Populate info and return a bulk copy meta for copying the data to device
            virtual BulkCopyMeta prepareCopy(PerVertexBuffer* buffer) const = 0;
            // Indicate whether the buffer contains index data too
            virtual bool hasIndices() const = 0;
        };

        // Meshes do not share indices, with variable vertices.
        class NoIndexBufferMeta : public BufferDataMeta {
        public:
            explicit NoIndexBufferMeta(std::vector<VertexDataMeta>&& perVertex) : perMeshVertices(perVertex) {}

            BulkCopyMeta prepareCopy(PerVertexBuffer* buffer) const override;
            bool hasIndices() const override { return false; };
        private:
            const std::vector<VertexDataMeta> perMeshVertices;
        };

        // Meshes share indices, with static vertices.
        class SharedIndexMeta : public BufferDataMeta {
        public:
            SharedIndexMeta(int meshes, const VertexDataMeta& perVertex, const VertexDataMeta& sharedIndices) : meshes(meshes),
                                                                                                                perMeshVertex(perVertex),
                                                                                                                sharedIndices(sharedIndices) {}
            BulkCopyMeta prepareCopy(PerVertexBuffer* buffer) const override;
            bool hasIndices() const override { return true; };
        private:
            const int meshes;
            const VertexDataMeta perMeshVertex;
            const VertexDataMeta sharedIndices;
        };

        // Meshes do not share indexes, with variable indices and vertices.
        class NoShareMeta : public BufferDataMeta {
        public:
            struct PerMesh {
                VertexDataMeta indices;
                VertexDataMeta vertices;
            };

            explicit NoShareMeta(std::vector<PerMesh>&& perMesh) : perMeshMeta(std::move(perMesh)) {}

            BulkCopyMeta prepareCopy(PerVertexBuffer* buffer) const override;
            bool hasIndices() const override { return true; };
        private:
            const std::vector<PerMesh> perMeshMeta;
        };

        PerVertexBuffer(const PerVertexBuffer&) = delete;
        PerVertexBuffer& operator=(const PerVertexBuffer&) = delete;

        // Render mesh a given number of times, into a recording buffer.
        void draw(const VkCommandBuffer& buffer, uint32_t bind, int index, uint32_t instances) const;

    protected:
        using VertexBuffer::VertexBuffer;

        // Stores vertex data for buffers without indices
        struct MeshDataNoIndex {
            struct Info {
                uint32_t vertexCount;
                VkDeviceSize vertexStart;
            };

            std::vector<Info> info;
        };

        // Stores vertex and index data for buffers with both
        struct MeshDataIndex {
            struct Info {
                uint32_t indexCount;
                VkDeviceSize indexStart;
                VkDeviceSize vertexStart;
            };

            std::vector<Info> info;
        };

        std::variant<MeshDataNoIndex, MeshDataIndex>* getInfo() { return &meshDataInfo; }

    private:

        std::variant<MeshDataNoIndex, MeshDataIndex> meshDataInfo;
    };

    // Stores static data for one-time upload.
    class StaticPerVertexBuffer : public PerVertexBuffer {
    public:
        StaticPerVertexBuffer(const BufferDataMeta& info, std::vector<VkVertexInputAttributeDescription>&& attrs);

        StaticPerVertexBuffer(const StaticPerVertexBuffer&) = delete;
        StaticPerVertexBuffer& operator=(const StaticPerVertexBuffer&) = delete;
    };

    // Stores host-visible data that can be reallocated.
    class DynamicPerVertexBuffer : public PerVertexBuffer, public DynamicBuffer {
    public:
        DynamicPerVertexBuffer(size_t size, std::vector<VkVertexInputAttributeDescription>&& attrs) : PerVertexBuffer(std::move(attrs)), DynamicBuffer(size, true, this) {}

        DynamicPerVertexBuffer(const DynamicPerVertexBuffer&) = delete;
        DynamicPerVertexBuffer& operator=(const DynamicPerVertexBuffer&) = delete;

        void copyToDevice(const BufferDataMeta& meta);
    };

    /*******************************************/
    /**                                        */
    /**       END OF PER-VERTEX BUFFERS        */
    /**                                        */
    /*******************************************/

    // Root class of buffers that store vertex data per instance of a mesh.
    class PerInstanceVertexBuffer : public VertexBuffer {
    public:
        PerInstanceVertexBuffer(const PerInstanceVertexBuffer&) = delete;
        PerInstanceVertexBuffer& operator=(const PerInstanceVertexBuffer&) = delete;

        void bind(const VkCommandBuffer& commands, uint32_t bindPoint, int offset) const;

        uint32_t getSize() const { return sizePerInstance; }

    protected:
        PerInstanceVertexBuffer(uint32_t size, std::vector<VkVertexInputAttributeDescription>&& attrs) : VertexBuffer(std::move(attrs)), sizePerInstance(size) {}
    private:
        const uint32_t sizePerInstance;
    };

    // Stores vertices that are static per instance of the mesh.
    class StaticPerInstanceBuffer : public PerInstanceVertexBuffer {
    public:
        StaticPerInstanceBuffer(uint32_t size, const void* data, uint32_t instances, std::vector<VkVertexInputAttributeDescription>&& attrs);

        template <typename C>
        StaticPerInstanceBuffer(const C& cont, std::vector<VkVertexInputAttributeDescription>&& attrs) : StaticPerInstanceBuffer(sizeof(cont[0]), cont.data(), CONTAINER_SIZE(cont), std::move(attrs)) {}

        StaticPerInstanceBuffer(const StaticPerInstanceBuffer&) = delete;
        StaticPerInstanceBuffer& operator=(const StaticPerInstanceBuffer&) = delete;
    };

    // Stores vertices of meshes that are dynamic (ie. text, shifting meshes
    class DynamicPerInstanceBuffer : public PerInstanceVertexBuffer, public DynamicBuffer {
    public:
        DynamicPerInstanceBuffer(uint32_t size, size_t maxInstances, std::vector<VkVertexInputAttributeDescription>&& attrs) : PerInstanceVertexBuffer(size, std::move(attrs)), DynamicBuffer(size * maxInstances, false, this) {}

        DynamicPerInstanceBuffer(const DynamicPerInstanceBuffer&) = delete;
        DynamicPerInstanceBuffer& operator=(const DynamicPerInstanceBuffer*) = delete;

        void copyToDevice(const void* data, uint32_t instances);

        template <typename C>
        void copyToDevice(const C& cont) {
            copyToDevice(cont.data(), CONTAINER_SIZE(cont));
        }
    };

    /*******************************************/
    /**                                        */
    /**      END OF PER-INSTANCE BUFFERS       */
    /**                                        */
    /*******************************************/

    // Holds uniform data on host and device.
    // Supports superallocating (allocating more than one "set" of uniforms at once)
    // Data is stored on host and device simultaneously, so set the host data and flush it to the device.
    class UniformBuffer : public DataBuffer {
    public:
        UniformBuffer(size_t chunkSize, int chunks);

        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        ~UniformBuffer() override { delete data; }

        // Whether this buffer holds a single chunk (is not superallocated).
        // Simplifies certain algorithms significantly if you know this beforehand
        bool isSingle() const { return numChunks == 1; }

        // Get the data in the buffer, casted to the given type
        template <typename DataType>
        DataType* getData(int index) const {
            checkIndex(index);
            return reinterpret_cast<DataType*>(data + chunkSize * index);
        }

        // Upload (flush) the uniform to the GPU
        void upload(int index) const;
        void upload(int index, VkDeviceSize dataSize, VkDeviceSize start) const;

        static VkDescriptorType getDescriptorType() { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }

        VkDescriptorBufferInfo getDescriptorInfo(int index) const;

    private:
        void checkIndex(int index) const;

        char* data;
        const size_t chunkSize;
        const int numChunks;

        size_t chunkLength;
    };

    // A small, highly efficient buffer much like the Uniform Buffer.
    class PushConstant {
    public:
        // Size must be < 128.
        PushConstant(size_t size, int numFrames);

        PushConstant(const PushConstant&) = delete;
        PushConstant& operator=(const PushConstant&) = delete;

        ~PushConstant() { delete[] data; }

        // Whether this buffer holds a single chunk (is not superallocated).
        // Simplifies certain algorithms significantly if you know this beforehand
        bool isSingle() const { return numFrames == 1; }

        uint32_t getSize() const { return sizePerFrame; }

        // Get the data in the buffer, casted to the given type
        template <typename DataType>
        DataType* getData(int frame) const {
            checkIndex(frame);
            return reinterpret_cast<DataType*>(data + (sizePerFrame * frame));
        }

        VkPushConstantRange makeRange(VkShaderStageFlags stage) {
            return VkPushConstantRange { stage, 0, sizePerFrame };
        }

        // Upload (flush) the uniform to the GPU
        void upload(const VkCommandBuffer& commands, const VkPipelineLayout& pipelineLayout, int frame, uint32_t offset, VkShaderStageFlags stage) const;


    private:
        void checkIndex(int index) const;

        char* data;
        const uint32_t sizePerFrame;
        const int numFrames;
    };

}