#include "vlkx/vulkan/abstraction/Image.h"

#include <cmath>
#include <fstream>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vlkx/vulkan/VulkanModule.h"
#include "util/File.h"

namespace vlkx {
    struct ImageConfig {
        explicit ImageConfig(bool readable = false) {
            if (readable) {
                tiling = VK_IMAGE_TILING_LINEAR;
                layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
            } else {
                tiling = VK_IMAGE_TILING_OPTIMAL;
                layout = VK_IMAGE_LAYOUT_UNDEFINED;
            }
        }

        uint32_t mipping = 1;
        uint32_t layers = 1;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkImageTiling tiling;
        VkImageLayout layout;
    };

    struct ImageData {
        ImageDescriptor::Dimension dimensions;
        const char *data;
    };

    ImageData loadImage(const std::string &path, int wantedChannels) {
        shadowutil::FileData *data = shadowutil::loadFile(path);
        int width, height, channels;

        stbi_uc *stbData = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(data->data.data()),
                                                 data->size,
                                                 &width,
                                                 &height,
                                                 &channels,
                                                 wantedChannels);
        if (stbData == nullptr)
            throw std::runtime_error("Unable to read image file " + std::string(path));

        switch (channels) {
            case 1:
            case 4:break;

            case 3: {
                stbi_image_free(stbData);
                stbData = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(data->data.data()),
                                                data->size,
                                                &width,
                                                &height,
                                                &channels,
                                                STBI_rgb_alpha);
                break;
            }

            default:
                throw std::runtime_error(
                    "Trying to load image with unsupported number of channels: " + std::to_string(channels));
        }

        return {{static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels)},
                reinterpret_cast<const char *>(stbData)};
    }

    std::optional<VkFormat> findFormatWith(const std::vector<VkFormat> &formats, VkFormatFeatureFlags feature) {
        for (const auto format : formats) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(VulkanModule::getInstance()->getDevice()->physical, format, &props);
            if ((props.optimalTilingFeatures & feature) == feature) return format;
        }

        return std::nullopt;
    }

    VkFormat findFormatForChannels(int channels, const std::vector<ImageUsage> &usages, bool highPrecision = false) {
        switch (channels) {
            case 1: {
                VkFormat best, alternative;
                if (highPrecision) {
                    best = VK_FORMAT_R16_SFLOAT;
                    alternative = VK_FORMAT_R16G16B16A16_SFLOAT;
                } else {
                    best = VK_FORMAT_R8_UNORM;
                    alternative = VK_FORMAT_R8G8B8A8_UNORM;
                }

                if (findFormatWith({best}, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT).has_value())
                    return best;
                else
                    return alternative;
            }

            case 4:
                if (highPrecision)
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
                else
                    return VK_FORMAT_R8G8B8A8_UNORM;
            default:
                throw std::runtime_error("Attempting to find format for invalid channels (RGB images have 4 channels!)");
        }
    }

    VkFormat findFormatForDepthStencil() {
        const auto format = findFormatWith({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                           VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        if (!format.has_value())
            throw std::runtime_error("Unable to find a format for a depth stencil image.");
        return format.value();
    }

    VkSampleCountFlagBits getMaxSamples(VkSampleCountFlags samples) {
        for (const auto count : {VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_16_BIT,
                                 VK_SAMPLE_COUNT_8_BIT, VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT,
                                 VK_SAMPLE_COUNT_1_BIT}) {
            if (samples & count)
                return count;
        }

        throw std::runtime_error("Multisampling isn't supported?");
    }

    Buffer::Buffer() {
    }

    ImageDescriptor Image::loadCubeFromDisk(const std::string &directory, const std::array<std::string, 6> &files,
                                            bool flipY) {
        stbi_set_flip_vertically_on_load(flipY);
        auto firstImage = loadImage(directory + "/" + files[0], STBI_default);
        const ImageDescriptor::Dimension &dim = firstImage.dimensions;
        char *data = new char[dim.getSize() * 6]; // TODO: Figure out how to make this delete
        memcpy(data, firstImage.data, dim.getSize());
        for (size_t i = 1; i < 6; i++) {
            auto image = loadImage(directory + "/" + files[1], STBI_default);
            if (!(image.dimensions.width == dim.width && image.dimensions.height == dim.height
                && image.dimensions.channels == dim.channels))
                throw std::runtime_error("Image " + std::to_string(i) + "(" + directory + "/" + files[i]
                                             + ") has different dimensions from the first image.");

            memcpy(data + i * dim.getSize(), image.data, dim.getSize());
        }

        stbi_set_flip_vertically_on_load(false);
        return {ImageDescriptor::Type::Cubemap, firstImage.dimensions, data};
    }

    ImageDescriptor Image::loadSingleFromDisk(std::string path, bool flipY) {
        stbi_set_flip_vertically_on_load(flipY);
        auto image = loadImage(std::move(path), STBI_default);
        stbi_set_flip_vertically_on_load(false);

        return {ImageDescriptor::Type::Single, image.dimensions, image.data};
    }

    TextureImage::Meta createTextureMeta(const ImageDescriptor &image, const std::vector<ImageUsage> &usages) {
        return TextureImage::Meta{
            image.getData(), usages,
            findFormatForChannels(image.getChannels(), usages),
            image.getWidth(), image.getHeight(), image.getChannels(),
        };
    }

    VkTools::ManagedImage createImage(const ImageConfig &config,
                                      VkImageCreateFlags flags,
                                      VkFormat format,
                                      const VkExtent3D &extent,
                                      VkImageUsageFlags usage) {
        const auto device = VulkanModule::getInstance()->getDevice();

        uint32_t graphicsQueue = (uint32_t) device->getQueues().graphics;
        VkImageCreateInfo info{
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr,
            flags, VK_IMAGE_TYPE_2D, format,
            extent, config.mipping, config.layers, config.samples, config.tiling,
            usage, VK_SHARING_MODE_EXCLUSIVE,
            1, &graphicsQueue, config.layout
        };

        // Prepare the managed image
        VkTools::ManagedImage image{};

        // Set up image allocation
        VmaAllocationCreateInfo allocateInfo = {};
        allocateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        // Allocate + create the image
        vmaCreateImage(VulkanModule::getInstance()->getAllocator(),
                       &info,
                       &allocateInfo,
                       &image.image,
                       &image.allocation,
                       nullptr);

        return image;
    }

    void waitForBarrier(const VkCommandBuffer &commands,
                        const VkImageMemoryBarrier &barrier,
                        const std::array<VkPipelineStageFlags, 2> &stages) {
        vkCmdPipelineBarrier(commands, stages[0], stages[1], 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    void transitionImage(const VkImage &image,
                         const ImageConfig &config,
                         VkImageAspectFlags aspect,
                         const std::array<VkImageLayout, 2> &layouts,
                         const std::array<VkAccessFlags, 2> &access,
                         const std::array<VkPipelineStageFlags, 2> &stages) {
        VkTools::immediateExecute([&](const VkCommandBuffer &commands) {
            waitForBarrier(commands,
                           {
                               VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                               nullptr,
                               access[0], access[1],
                               layouts[0], layouts[1],
                               VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                               image,
                               {aspect, 0, config.mipping, 0, config.layers}
                           }, stages);
        }, VulkanModule::getInstance()->getDevice());
    }

    inline VkOffset3D convertExtent(const VkExtent2D &extent) {
        return VkOffset3D{static_cast<int32_t>(extent.width), static_cast<int32_t>(extent.height), 1};
    }

    inline VkExtent3D expandExtent(const VkExtent2D &extent) {
        return VkExtent3D{extent.width, extent.height, 1};
    }

    std::vector<VkExtent2D> getExtentForMipmaps(const VkExtent3D &extent) {
        const int largest = std::max(extent.width, extent.height);
        const int mipping = std::floor(std::log2(largest));
        std::vector<VkExtent2D> extents(mipping);
        VkExtent2D imageExt{extent.width, extent.height};
        for (size_t level = 0; level < mipping; ++level) {
            imageExt.width = imageExt.width > 1 ? imageExt.width / 2 : 1;
            imageExt.height = imageExt.height > 1 ? imageExt.height / 2 : 1;
            extents[level] = imageExt;
        }

        return extents;
    }

    void generateMipmaps(const VkImage &image,
                         VkFormat format,
                         const VkExtent3D &extent,
                         const std::vector<VkExtent2D> &mipExtents) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(VulkanModule::getInstance()->getDevice()->physical, format, &props);
        if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
            throw std::runtime_error("Attempting to create Mipmaps for an image format that does not support blitting");

        uint32_t destLevel = 1;
        VkExtent2D previousExt{extent.width, extent.height};

        VkTools::immediateExecute([&](const VkCommandBuffer &commands) {
            // Blit the new images into place
            for (const auto &ext : mipExtents) {
                const uint32_t sourceLevel = destLevel - 1;
                VkImageMemoryBarrier barrier{
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    0, 0, image, {VK_IMAGE_ASPECT_COLOR_BIT, sourceLevel, 1, 0, 1}
                };

                waitForBarrier(commands, barrier, {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT});

                const VkImageBlit blit{
                    {VK_IMAGE_ASPECT_COLOR_BIT, sourceLevel, 0, 1},
                    {{0, 0, 0}, convertExtent(previousExt)},
                    {VK_IMAGE_ASPECT_COLOR_BIT, destLevel, 0, 1},
                    {{0, 0, 0}, convertExtent(ext)}
                };
                vkCmdBlitImage(commands, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

                ++destLevel;
                previousExt = ext;
            }
        }, VulkanModule::getInstance()->getDevice());

        VkTools::immediateExecute([&](const VkCommandBuffer &commands) {
            // Convert all images to shader read only so we can sample them
            for (uint32_t level = 0; level < mipExtents.size() + 1; ++level) {
                VkImageMemoryBarrier barrier{
                    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                    level == mipExtents.size() ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                                               : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    0, 0, image, {VK_IMAGE_ASPECT_COLOR_BIT, level, 1, 0, 1}
                };

                waitForBarrier(commands, barrier, {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT});
            }
        }, VulkanModule::getInstance()->getDevice());
    }

    Image::Image(const VkExtent2D &ext, VkFormat form) : extent(ext), format(form) {
        dev = VulkanModule::getInstance()->getDevice();
    }

    void ImageStagingBuffer::copy(const VkImage &target, const VkExtent3D &extent, uint32_t layers) const {
        VkTools::immediateExecute([&](const VkCommandBuffer &commands) {
            const VkBufferImageCopy copyData{0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layers}, {0, 0, 0}, extent};
            vkCmdCopyBufferToImage(commands, getBuffer(), target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyData);
        }, VulkanModule::getInstance()->getDevice());
    }

    ImageSampler::ImageSampler(int mipLevels, const vlkx::ImageSampler::Config &config)
        : sampler(VkTools::createSampler(config.filter,
                                         config.mode,
                                         mipLevels,
                                         VulkanModule::getInstance()->getDevice()->logical)) {
        dev = VulkanModule::getInstance()->getDevice();
    }

    Buffer::BulkCopyMeta TextureImage::Meta::getCopyMeta() const {
        const VkDeviceSize singleSize = width * height * channels;
        const VkDeviceSize totalSize = singleSize * data.size();
        std::vector<Buffer::CopyMeta> copy(data.size());

        // If we're making a framebuffer, we have no data to copy in.
        if (data[0] == nullptr)
            return {totalSize, {}};

        for (size_t i = 0; i < copy.size(); ++i) {
            copy[i] = {data[i], singleSize, singleSize * i};
        }

        return {totalSize, std::move(copy)};
    }

    TextureImage::TextureImage(bool mipmapping,
                               const ImageSampler::Config &samplerConfig,
                               const vlkx::TextureImage::Meta &meta)
        : Image(meta.getExtent(), meta.format), buffer(mipmapping, meta), sampler(buffer.getMipping(), samplerConfig) {

        setView(VkTools::createImageView(buffer.getImage(),
                                         format,
                                         VK_IMAGE_ASPECT_COLOR_BIT,
                                         buffer.getMipping(),
                                         meta.data.size(),
                                         VulkanModule::getInstance()->getDevice()->logical));
    }

    TextureImage::TextureImage(bool mipmapping,
                               const ImageDescriptor &image,
                               const std::vector<ImageUsage> &usages,
                               const ImageSampler::Config &config)
        : TextureImage(mipmapping, config, createTextureMeta(image, usages)) {}

    TextureImage::TextureBuffer::TextureBuffer(bool mipmaps, const vlkx::TextureImage::Meta &meta) : ImageBuffer() {
        const VkExtent3D extent = expandExtent(meta.getExtent());
        const auto layers = meta.data.size();
        if (layers != 1 && layers != 6)
            throw std::runtime_error(
                "Attempting to allocate a texture buffer for an invalid number of textures; only single textures and cubemap textures are supported.");

        ImageConfig config;
        config.layers = meta.data.size();

        std::vector<VkExtent2D> mipExtents;
        if (mipmaps) {
            mipExtents = getExtentForMipmaps(extent);
            mipLevels = mipExtents.size() + 1;
        }

        config.mipping = mipLevels;

        VkImageCreateFlags createFlags{};
        if (layers == 6)
            createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        auto usage = ImageUsage::getFlagsForUsage(meta.usages);
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (mipmaps) usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        setImage(createImage(config, createFlags, meta.format, extent, usage));

        transitionImage(getImage(),
                        config,
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        {VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
                        {0, VK_ACCESS_TRANSFER_WRITE_BIT},
                        {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT});

        const ImageStagingBuffer staging(meta.getCopyMeta());
        staging.copy(getImage(), extent, config.layers);

        if (mipmaps) {
            generateMipmaps(getImage(), meta.format, extent, mipExtents);
        } else {
            transitionImage(getImage(),
                            config,
                            VK_IMAGE_ASPECT_COLOR_BIT,
                            {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                            {VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT},
                            {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT});
        }
    }

    RefCountedTexture::ReferenceCounter RefCountedTexture::get(const vlkx::RefCountedTexture::ImageLocation &location,
                                                               const std::vector<ImageUsage> &usages,
                                                               const ImageSampler::Config &config) {
        bool mips;
        const std::string *ident;
        std::unique_ptr<ImageDescriptor> image;

        if (const auto *singleTex = std::get_if<std::string>(&location); singleTex != nullptr) {
            mips = true;
            ident = singleTex;
            image = std::make_unique<ImageDescriptor>(Image::loadSingleFromDisk(*singleTex, false));
        } else if (const auto *cubeTex = std::get_if<CubemapLocation>(&location); cubeTex != nullptr) {
            mips = false;
            ident = &cubeTex->directory;
            image =
                std::make_unique<ImageDescriptor>(Image::loadCubeFromDisk(cubeTex->directory, cubeTex->files, false));
        }

        return ReferenceCounter::get(*ident, mips, config, createTextureMeta(*image, usages));
    }

    DepthStencilImage::DepthStencilImage(const VkExtent2D &extent) : Image(extent, findFormatForDepthStencil()),
                                                                     buffer(extent, format) {
        setView(VkTools::createImageView(getImage(),
                                         format,
                                         VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                         1,
                                         1,
                                         VulkanModule::getInstance()->getDevice()->logical));
    }

    DepthStencilImage::DepthStencilBuffer::DepthStencilBuffer(const VkExtent2D &extent, VkFormat format)
        : ImageBuffer() {
        setImage(createImage(ImageConfig{},
                             0,
                             format,
                             expandExtent(extent),
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT));
    }

    SwapchainImage::SwapchainImage(const VkImage &image, const VkExtent2D &extent, VkFormat format) : Image(extent,
                                                                                                            format),
                                                                                                      image(image) {
        setView(VkTools::createImageView(image,
                                         format,
                                         VK_IMAGE_ASPECT_COLOR_BIT,
                                         1,
                                         1,
                                         VulkanModule::getInstance()->getDevice()->logical));
        managed = {image, nullptr};
    }

    std::unique_ptr<Image> MultisampleImage::createColor(const vlkx::Image &targetImage,
                                                         vlkx::MultisampleImage::Mode mode) {
        return std::unique_ptr<Image>(new MultisampleImage(targetImage.getExtent(),
                                                           targetImage.getFormat(),
                                                           mode,
                                                           MultisampleBuffer::Type::Color));
    }

    std::unique_ptr<Image> MultisampleImage::createDepthStencilMS(const VkExtent2D &extent,
                                                                  vlkx::MultisampleImage::Mode mode) {
        return std::unique_ptr<Image>(new MultisampleImage(extent,
                                                           findFormatForDepthStencil(),
                                                           mode,
                                                           MultisampleBuffer::Type::DepthStencil));
    }

    std::unique_ptr<Image> MultisampleImage::createDepthStencil(VkExtent2D &extent, std::optional<Mode> mode) {
        if (mode.has_value())
            return createDepthStencilMS(extent, mode.value());
        else
            return std::make_unique<DepthStencilImage>(extent);
    }

    MultisampleImage::MultisampleImage(const VkExtent2D &extent,
                                       VkFormat format,
                                       vlkx::MultisampleImage::Mode mode,
                                       MultisampleBuffer::Type type)
        : Image(extent, format), samples(chooseSamples(mode)), buffer(type, extent, format, samples) {

        VkImageAspectFlags aspect;
        switch (type) {
            case MultisampleBuffer::Type::Color: aspect = VK_IMAGE_ASPECT_COLOR_BIT;
                break;
            case MultisampleBuffer::Type::DepthStencil:aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                break;
        }
        setView(VkTools::createImageView(buffer.getImage(),
                                         format,
                                         aspect,
                                         1,
                                         1,
                                         VulkanModule::getInstance()->getDevice()->logical));
    }

    VkSampleCountFlagBits MultisampleImage::chooseSamples(vlkx::MultisampleImage::Mode mode) {
        VkPhysicalDeviceLimits limits;
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(VulkanModule::getInstance()->getDevice()->physical, &props);
        limits = props.limits;

        const VkSampleCountFlags sampleFlags =
            std::min({limits.framebufferColorSampleCounts, limits.framebufferDepthSampleCounts,
                      limits.framebufferStencilSampleCounts});
        const VkSampleCountFlagBits maxSamples = getMaxSamples(sampleFlags);
        switch (mode) {
            case Mode::MostEfficient: return std::min(VK_SAMPLE_COUNT_4_BIT, maxSamples);
            case Mode::Highest: return maxSamples;
        }
        throw std::runtime_error("Invalid multisample mode");
    }

    MultisampleImage::MultisampleBuffer::MultisampleBuffer(vlkx::MultisampleImage::MultisampleBuffer::Type type,
                                                           const VkExtent2D &extent,
                                                           VkFormat format,
                                                           VkSampleCountFlagBits samples)
        : ImageBuffer() {

        VkImageUsageFlags usageFlags;
        switch (type) {
            case Type::Color:usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
                break;
            case Type::DepthStencil: usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                break;
        }

        ImageConfig config;
        config.samples = samples;

        setImage(createImage(config, 0, format, expandExtent(extent), usageFlags));

    }

}