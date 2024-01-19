#pragma once

#include <optional>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

namespace vlkx {
    /**
     * Describes how an image (collection of color data) will be used in the GPU.
     * Has three parts; an overall description, access methods, and location of access.
     * Use the static methods to create an instance, or the blank initializer and set fields as required.
     */
    class ImageUsage {
      public:
        enum class Type {
            DontCare,           // Image is unused
            RenderTarget,       // Color attachment is used
            DepthStencil,       // Depth / Stencil buffer is used
            Multisample,        // Resolves to a multisampled image
            Presentation,       // Framebuffer for presentation
            LinearAccess,       // Host-Readable
            InputAttachment,    // Only what we write, is read.
            Sampled,            // Image is sampled (ie. texture)
            Transfer            // Image is used as an intermediate for transfer.
        };

        enum class Access {
            DontCare,           // Image is unused
            ReadOnly,           // Read Only access.
            WriteOnly,          // Write Only access.
            ReadWrite,          // Read/Write access.
        };

        enum class Location {
            DontCare,           // Image is unused
            Host,               // Image only exists on the host and will be transferred.
            VertexShader,       // Image is only used in the VertexAll Shader.
            FragmentShader,     // Image is only used in the Fragment Shader.
            ComputeShader,      // Image is only used in a Compute Shader.
            Other,              // Reserved.
        };

        // Sampled in a fragment shader. Read-Only.
        static ImageUsage sampledFragment() { return {Type::Sampled, Access::ReadOnly, Location::FragmentShader}; };

        // Used as a render target (a render pass will output to this image). Read/Write.
        static ImageUsage renderTarget(int loc) {
            return {Type::RenderTarget, Access::ReadWrite, Location::Other, loc};
        };

        // Resolves to a multisampled image. Write-Only.
        static ImageUsage multisample() { return {Type::Multisample, Access::WriteOnly, Location::Other}; };

        // A depth or stencil buffer. Access is given, but must not be DontCare.
        static ImageUsage depthStencil(Access acc) { return {Type::DepthStencil, acc, Location::Other}; };

        // Used to show to the user. Write-Only.
        static ImageUsage presentation() { return {Type::Presentation, Access::ReadOnly, Location::Other}; };

        // Input attachment for a fragment shader. Usually a texture. Read-Only.
        static ImageUsage input() { return {Type::InputAttachment, Access::ReadOnly, Location::FragmentShader}; };

        // Linearly accessed image. For a compute shader. Access is given, but must not be DontCare.
        static ImageUsage compute(Access acc) { return {Type::LinearAccess, acc, Location::ComputeShader}; };

        explicit ImageUsage() : ImageUsage(Type::DontCare, Access::DontCare, Location::DontCare) {}

        bool operator==(const ImageUsage &other) const {
            return type == other.type && access == other.access && location == other.location;
        }

        VkImageUsageFlagBits getUsageFlags() const {
            switch (type) {
                case Type::DontCare: throw std::runtime_error("No usage for type DontCare");
                case Type::RenderTarget:
                case Type::Multisample:
                case Type::Presentation:return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                case Type::DepthStencil:return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                case Type::LinearAccess:return VK_IMAGE_USAGE_STORAGE_BIT;
                case Type::Sampled:return VK_IMAGE_USAGE_SAMPLED_BIT;
                case Type::Transfer:
                    switch (access) {
                        case Access::DontCare: throw std::runtime_error("No access type specified for transfer usage.");
                        case Access::ReadOnly: return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                        case Access::WriteOnly: return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                        case Access::ReadWrite:
                            throw std::runtime_error("ReadWrite access type for Transfer usage is invalid.");
                    }
            }
            throw std::runtime_error("Invalid image usage.");
        }

        static VkImageUsageFlags getFlagsForUsage(const std::vector<ImageUsage> &usages) {
            auto flags = 0;
            for (const auto &usage : usages) {
                if (usage.type != Type::DontCare)
                    flags |= usage.getUsageFlags();
            }

            return static_cast<VkImageUsageFlags>(flags);
        }

        VkImageLayout getLayout() const {
            switch (type) {
                case Type::DontCare: return VK_IMAGE_LAYOUT_UNDEFINED;
                case Type::RenderTarget:
                case Type::Multisample:return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                case Type::DepthStencil: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                case Type::Presentation: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                case Type::LinearAccess: return VK_IMAGE_LAYOUT_GENERAL;
                case Type::Sampled: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                case Type::Transfer:
                    return access == Access::ReadOnly ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                                      : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                case Type::InputAttachment:break;
            }
            throw std::runtime_error("Invalid image layout.");
        }

        VkPipelineStageFlags getStage() const {
            switch (type) {
                case Type::DontCare: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                case Type::RenderTarget:
                case Type::Multisample:
                case Type::Presentation:return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                case Type::DepthStencil:
                    return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                case Type::LinearAccess:
                case Type::Sampled:
                    switch (location) {
                        case Location::Host: return VK_PIPELINE_STAGE_HOST_BIT;
                        case Location::FragmentShader: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        case Location::ComputeShader: return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

                        case Location::VertexShader:
                        case Location::Other:
                            throw std::runtime_error(
                                "Linear or sampled attachments must not be used in VertexAll or Other stages.");
                        case Location::DontCare:
                            throw std::runtime_error("Linear or sampled attachments must have an access.");
                    }

                case Type::Transfer:return VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            throw std::runtime_error("Invalid pipeline stage.");
        }

        Access getAccess() const { return access; }

        Type getType() const { return type; }

        VkAccessFlags getAccessFlags() const {
            switch (type) {
                case Type::DontCare: return VK_ACCESS_NONE_KHR;
                case Type::RenderTarget:
                    return getReadOrWrite(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
                case Type::DepthStencil:
                    return getReadOrWrite(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
                case Type::Multisample: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                case Type::Presentation: return 0;
                case Type::LinearAccess:
                case Type::Sampled:
                    return location == Location::Host ? getReadOrWrite(VK_ACCESS_HOST_READ_BIT,
                                                                       VK_ACCESS_HOST_WRITE_BIT) : getReadOrWrite(
                        VK_ACCESS_SHADER_READ_BIT,
                        VK_ACCESS_SHADER_WRITE_BIT);
                case Type::Transfer:return getReadOrWrite(VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
                case Type::InputAttachment:return 0;
            }
            throw std::runtime_error("Invalid image access type.");
        };

      private:

        ImageUsage(Type t, Access a, Location l, std::optional<int> att = std::nullopt)
            : type(t), access(a), location(l), attachment(att) {}

        Type type;
        Access access;
        Location location;
        std::optional<int> attachment;

        inline VkAccessFlags getReadOrWrite(VkAccessFlags read, VkAccessFlags write) const {
            VkAccessFlags flag = 0;
            if (access == Access::ReadOnly || access == Access::ReadWrite)
                flag |= read;
            if (access == Access::WriteOnly || access == Access::ReadWrite)
                flag |= write;
            return flag;
        };
    };

    /**
     * Describes how a single image will be used in each stage of a render pass.
     * Allows a single image to be written to during one pass, read in a second, and presented in the final, and tracked.
     * Helps figure out the optimizations that Vulkan can do to this image and the render passes that use it.
     */
    class UsageTracker {
      public:

        explicit UsageTracker(const ImageUsage &initial) : initialUsage(initial) {}

        explicit UsageTracker() = default;

        UsageTracker(UsageTracker &&) noexcept = default;

        UsageTracker &operator=(UsageTracker &&) noexcept = default;

        // Fluent API; chain calls in a builder pattern.
        #define fluent UsageTracker&

        fluent add(int pass, const ImageUsage &usage) {
            usageAtSubpass.insert({pass, usage});

            return *this;
        }

        fluent add(int start, int end, const ImageUsage &usage) {
            for (int subpass = start; subpass <= end; ++subpass)
                add(subpass, usage);

            return *this;
        }

        fluent addMultisample(int pass, std::string_view name) {
            multisamples.insert({pass, std::string(name)});
            return add(pass, ImageUsage::multisample());
        }

        fluent setFinal(const ImageUsage &usage) {
            finalUsage = usage;
            return *this;
        }

        [[nodiscard]] std::vector<ImageUsage> getUsages() const {
            size_t count = usageAtSubpass.size() + (finalUsage.has_value() ? 1 : 0) + 1;

            std::vector<ImageUsage> usages;
            usages.reserve(count);
            usages.emplace_back(initialUsage);

            for (const auto &pair : usageAtSubpass)
                usages.emplace_back(pair.second);

            if (finalUsage.has_value())
                usages.emplace_back(finalUsage.value());

            return usages;
        }

        [[nodiscard]] const std::map<int, ImageUsage> &getUsageMap() const {
            return usageAtSubpass;
        }

        ImageUsage &getInitialUsage() { return initialUsage; }

        std::optional<ImageUsage> getFinalUsage() { return finalUsage; }

      private:

        std::map<int, ImageUsage> usageAtSubpass;
        ImageUsage initialUsage;
        std::optional<ImageUsage> finalUsage;
        std::map<int, std::string> multisamples;
    };

    /**
     * A simple wrapper that allows tracking the current usage of multiple images.
     */
    class MultiImageTracker {
      public:
        MultiImageTracker() = default;

        MultiImageTracker(const MultiImageTracker &) = delete;

        MultiImageTracker &operator=(const MultiImageTracker &) = delete;

        // Fluent API; chain calls in a builder pattern
        #undef fluent
        #define fluent MultiImageTracker&

        fluent track(std::string &&name, const ImageUsage &usage) {
            images.insert({std::move(name), usage});
            return *this;
        }

        fluent track(const std::string &name, const ImageUsage &usage) {
            return track(std::string(name), usage);
        }

        fluent update(const std::string &name, const ImageUsage &usage) {
            auto iter = images.find(name);
            iter->second = usage;
            return *this;
        }

        [[nodiscard]] bool isTracking(const std::string &image) const {
            return images.contains(image);
        }

        [[nodiscard]] const ImageUsage &get(const std::string &image) const {
            return images.at(image);
        }

      private:
        std::map<std::string, ImageUsage> images;

    };
}