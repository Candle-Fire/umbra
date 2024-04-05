#pragma once

#include "vlkx/vulkan/Tools.h"
#include "ImageUsage.h"
#include "shadow/util/RefCounter.h"
#include "Buffer.h"
#include <array>

#include <utility>

namespace vlkx {

    // Describes an image without initializing or storing any heavy data.
    class ImageDescriptor {
    public:
        enum class Type { Single, Cubemap };
        struct Dimension {
            uint32_t width;
            uint32_t height;
            uint32_t channels;

            VkExtent2D getExtent() const { return { width, height }; }
            size_t getSize() const { return width * height * channels; }
        };

        Type getType() const { return type; }
        VkExtent2D getExtent() const { return dimensions.getExtent(); }
        uint32_t getWidth() const { return dimensions.width; }
        uint32_t getHeight() const { return dimensions.height; }
        uint32_t getChannels() const { return dimensions.channels; }

        std::vector<void*> getData() const {
            if (type == Type::Single) return { (void*) data };
            std::vector<void*> dataPtrs;
            dataPtrs.reserve(6);

            size_t offset = 0;
            for (size_t i = 0; i < 6; i++) {
                dataPtrs.emplace_back((char*) data + offset);
                offset += dimensions.getSize();
            }

            return dataPtrs;
        }

        int getLayers() const { return type == Type::Single ? 1 : 6; }

        ImageDescriptor(Type t, const Dimension& d, const void* ptr) : type(t), dimensions(d), data(ptr) {}

    private:
        Type type;
        Dimension dimensions;
        const void* data;

    };

    // A staging buffer specialized for uploading images.
    class ImageStagingBuffer : public StagingBuffer {
    public:
        using StagingBuffer::StagingBuffer;

        ImageStagingBuffer(const ImageStagingBuffer&) = delete;
        ImageStagingBuffer& operator=(const ImageStagingBuffer&) = delete;

        void copy(const VkImage& target, const VkExtent3D& extent, uint32_t layers) const;
    };

    // Root class that stores image data on GPU buffers
    class ImageBuffer : public Buffer {
    public:
        ImageBuffer(const ImageBuffer&) = delete;
        ImageBuffer& operator=(const ImageBuffer&) = delete;

        ~ImageBuffer() override {
            vmaDestroyImage(VkTools::allocator, image.image, image.allocation);
        }

        const VkTools::ManagedImage& get() const { return image; }
        const VkImage& getImage() const { return image.image; }

    protected:
        using Buffer::Buffer;

        void setImage(const VkTools::ManagedImage newImg) { image = newImg; }

    private:
        VkTools::ManagedImage image;

    };

    // Base class of all images; stores the common data.
    class Image {
    public:
        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;

        virtual ~Image() {
            vkDestroyImageView(dev->logical, view, nullptr);
        }

        static VkDescriptorType getSampleType() { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
        static VkDescriptorType getLinearType() { return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; }

        static ImageDescriptor loadSingleFromDisk(std::string path, bool flipY);
        // The following are left unimplemented intentionally.
        //static ImageDescriptor loadSingleFromVFS(std::string path, bool flipY);
        static ImageDescriptor loadCubeFromDisk(const std::string& directory, const std::array<std::string, 6>& files, bool flipY);
        //static ImageDescriptor loadCubeFromVFS(std::string directory, const std::array<std::string, 6>& files, bool flipY);

        virtual ImageUsage getUsage() const { return ImageUsage {}; }

        const VkTools::ManagedImage& operator*() const { return get(); }

        virtual const VkTools::ManagedImage& get() const = 0;
        virtual const VkImage& getImage() const = 0;

        const VkImageView& getView() const { return view; }
        const VkExtent2D& getExtent() const { return extent; }
        VkFormat getFormat() const { return format; }
        virtual VkSampleCountFlagBits getSamples() const { return VK_SAMPLE_COUNT_1_BIT; }


    protected:
        Image(const VkExtent2D& ext, VkFormat form);

        void setView(const VkImageView& imgView) { view = imgView; }

        VulkanDevice* dev;
        VkImageView view;
        VkExtent2D extent;
        VkFormat format;
        VkSampleCountFlagBits sampleCount;
    };

    // Configures image sampling in a sensible and extensible way
    class ImageSampler {
    public:
        struct Config {
            explicit Config(VkFilter filter = VK_FILTER_LINEAR, VkSamplerAddressMode mode = VK_SAMPLER_ADDRESS_MODE_REPEAT) : filter(filter), mode(mode) {}

            VkFilter filter;
            VkSamplerAddressMode mode;
        };

        ImageSampler(int mipLevels, const Config& config);

        ImageSampler(const ImageSampler&) = delete;
        ImageSampler& operator=(const ImageSampler&) = delete;

        ~ImageSampler() {
            vkDestroySampler(dev->logical, sampler, nullptr);
        }

        const VkSampler& operator*() const { return sampler; }

    private:
        VkSampler sampler;
        VulkanDevice* dev;
    };

    // Root of images which can be sampled.
    class SamplableImage {
    public:
        virtual ~SamplableImage() = default;

        // Return a VkDescriptorImageInfo we can use to update sets.
        virtual VkDescriptorImageInfo getInfo(VkImageLayout layout) const = 0;
        VkDescriptorImageInfo getInfoForSampling() const { return getInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); }
        VkDescriptorImageInfo getInfoForLinear() const { return getInfo(VK_IMAGE_LAYOUT_GENERAL); }
    };

    // A samplable image that lives on the GPU, with optional mipmapping.
    // Use a RefCountedTexture when loading from files.
    class TextureImage : public Image, public SamplableImage {
    public:
        // Image metadata
        struct Meta {
            VkExtent2D getExtent() const { return { width, height }; }
            VkExtent3D get3DExtent() const { return { width, height, channels }; }

            Buffer::BulkCopyMeta getCopyMeta() const;

            std::vector<void*> data;
            std::vector<ImageUsage> usages;
            VkFormat format;
            uint32_t width;
            uint32_t height;
            uint32_t channels;
        };

        TextureImage(bool mipmapping, const ImageSampler::Config& samplerConfig, const Meta& meta);
        TextureImage(bool mipmapping, const ImageDescriptor& image, const std::vector<ImageUsage>& usages, const ImageSampler::Config& config);

        TextureImage(const TextureImage&) = delete;
        TextureImage& operator=(const TextureImage&) = delete;

        const VkTools::ManagedImage& get() const override { return buffer.get(); }
        const VkImage& getImage() const override { return buffer.getImage(); }

        VkDescriptorImageInfo getInfo(VkImageLayout layout) const override {
            return { *sampler, getView(), layout };
        }
        // Textures are sampled in fragment shaders.
        ImageUsage getUsage() const override {
            return ImageUsage::sampledFragment();
        }
    private:
        class TextureBuffer : public ImageBuffer {
        public:
            TextureBuffer(bool mipmaps, const Meta& meta);

            TextureBuffer(const TextureBuffer&) = delete;
            TextureBuffer& operator=(const TextureBuffer&) = delete;

            int getMipping() const { return mipLevels; }
        private:
            int mipLevels = 1;
        };

        const TextureBuffer buffer;
        const ImageSampler sampler;
    };

    // A texture image that lives on GPU that is reference counted.
    // This allows it to be reused multiple times without reading the file in more than once.
    // It also allows the texture to be destructed automatically once nothing in the scene uses it, for eg. level changes.
    class RefCountedTexture : public SamplableImage {
    public:
        // Cubemaps are simply 6 textures, so we include them here for easy instantiation.
        struct CubemapLocation {
            std::string directory;
            std::array<std::string, 6> files;
        };

        // Reference Counting works on both individual files and cubemaps, so we put them together.
        using ImageLocation = std::variant<std::string, CubemapLocation>;

        RefCountedTexture(const ImageLocation& location, const std::vector<ImageUsage>& usages, const ImageSampler::Config& config)
            : texture(get(location, usages, config)) {}

        RefCountedTexture(RefCountedTexture&&) noexcept = default;
        RefCountedTexture& operator=(RefCountedTexture&&) noexcept = default;

        [[nodiscard]] VkDescriptorImageInfo getInfo(VkImageLayout layout) const override {
            return texture->getInfo(layout);
        }

        const Image* operator->() const { return texture.operator->(); }

    private:
        using ReferenceCounter = shadowutil::RefCounter<TextureImage>;
        // Get or load the specified image.
        static ReferenceCounter get(const ImageLocation& location, const std::vector<ImageUsage>& usages, const ImageSampler::Config& config);

        ReferenceCounter texture;
    };

    // TODO: unowned, offscreen images.

    // Image that can be used as a depth / stencil buffer attachment.
    class DepthStencilImage : public Image {
    public:
        DepthStencilImage(const DepthStencilImage&) = delete;
        DepthStencilImage& operator=(const DepthStencilImage&) = delete;

        DepthStencilImage(const VkExtent2D& extent);

        const VkTools::ManagedImage& get() const override { return buffer.get(); }
        const VkImage& getImage() const override { return buffer.getImage(); }

    private:
        class DepthStencilBuffer : public ImageBuffer {
        public:
            DepthStencilBuffer(const VkExtent2D& extent, VkFormat format);
            DepthStencilBuffer(const DepthStencilBuffer&) = delete;
            DepthStencilBuffer& operator=(const DepthStencilBuffer&) = delete;
        };

        const DepthStencilBuffer buffer;
    };

    // Image that references an existing image on the swapchain
    class SwapchainImage : public Image {
    public:
        SwapchainImage(const SwapchainImage&) = delete;
        SwapchainImage& operator=(const SwapchainImage&) = delete;

        SwapchainImage(const VkImage& image, const VkExtent2D& extent, VkFormat format);

        const VkTools::ManagedImage& get() const override { return managed; }
        const VkImage& getImage() const override { return image; }

    private:
        VkImage image;
        VkTools::ManagedImage managed;
    };

    class MultisampleImage : public Image {
    public:
        enum class Mode {
            MostEfficient,
            Highest
        };

        static std::unique_ptr<Image> createColor(const Image& targetImage, Mode mode);
        static std::unique_ptr<Image> createDepthStencilMS(const VkExtent2D& extent, Mode mode);
        static std::unique_ptr<Image> createDepthStencil(VkExtent2D& extent, std::optional<Mode> mode);

        const VkTools::ManagedImage& get() const override { return buffer.get(); }
        const VkImage& getImage() const override { return buffer.getImage(); }

        VkSampleCountFlagBits getSamples() const override { return samples; }
    private:
        class MultisampleBuffer : public ImageBuffer {
        public:
            enum class Type { Color, DepthStencil };

            MultisampleBuffer(Type type, const VkExtent2D& extent, VkFormat format, VkSampleCountFlagBits samples);
            MultisampleBuffer(const MultisampleBuffer&) = delete;
            MultisampleBuffer& operator=(const MultisampleBuffer&) = delete;
        };

        MultisampleImage(const VkExtent2D& extent, VkFormat format, Mode mode, MultisampleBuffer::Type type);

        VkSampleCountFlagBits chooseSamples(Mode mode);

        const VkSampleCountFlagBits samples;
        const MultisampleBuffer buffer;
    };

}