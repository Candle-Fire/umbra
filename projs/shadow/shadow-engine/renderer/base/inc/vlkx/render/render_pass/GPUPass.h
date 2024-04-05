#pragma once

#include <vulkan/vulkan.h>
#include "vlkx/vulkan/abstraction/ImageUsage.h"
#include <string>
#include <functional>
#include <memory>
#include "GenericRenderPass.h"

namespace vlkx {

    /**
     * The common base class for rendering and computing passes that run on the GPU.
     * Provides some utility methods for handling attachment metadata between subpasses.
     */
    class CommonPass {
      public:

        explicit CommonPass(int passes) : numPasses(passes) {}

        // Delete the copy and move constructors
        CommonPass(const CommonPass &) = delete;

        CommonPass &operator=(const CommonPass &) = delete;

        virtual ~CommonPass() = default;

        // Get the image layout of the given image at the start of this pass
        VkImageLayout getInitialLayout(const std::string &name) const;

        // Get the image layout of the given image at the end of this pass
        VkImageLayout getFinalLayout(const std::string &name) const;

        // Get the image layout of the given image before the given subpass starts
        VkImageLayout getSubpassLayout(const std::string &name, int subpass) const;

        // Update the state of the given image's usage tracker.
        void update(const std::string &name, MultiImageTracker &tracker) const;

      protected:
        /**
         * Some metadata about the usage of an image between subpasses.
         */
        struct Usages {
            Usages(const int last, const ImageUsage *prev, const ImageUsage *curr)
                : lastSubpass(last), lastUsage(*prev), currentUsage(*curr) {}

            const int lastSubpass;
            const ImageUsage &lastUsage;
            const ImageUsage &currentUsage;
        };

        // Add the usage of an image in the pass to its' tracker.
        void addUsage(std::string &&name, UsageTracker &&tracker);

        // Get the full history of the image's usages up to this rendering pass.
        const UsageTracker &getHistory(const std::string &name) const;

        // Get the usage of an image at the start of the given pass.
        const ImageUsage *getUsage(const std::string &name, int pass) const;

        // Retrieve image usage data, but only if the image is barriered at the given pass.
        std::optional<Usages> checkForSync(const std::string &name, int pass) const;

        // Validate that the subpass is valid for the given image.
        // The meaning of includeVirtual is defined by the child implementation.
        void validate(int pass, const std::string &image, bool includeVirtual) const;

        int getVirtualInitial() const { return -1; }

        int getVirtualFinal() const { return numPasses; }

      protected:
        std::map<std::string, UsageTracker> usageHistory;
        const int numPasses;
    };

    /**
     * The Common Pass implementation for Graphics passes; that is, render passes that output to color buffers
     *  for presentation to the screen, or to be used as textures in such.
     * The instance of the GraphicsPass can be stored and reused to create multiple RenderPassBuilders.
     * In this way it is essentially a RenderPassBuilderFactory.
     */
    class GraphicsPass : public CommonPass {
      public:

        using LocationGetter = std::function<int(int pass)>;

        explicit GraphicsPass(int passes) : CommonPass{passes} {}

        GraphicsPass(const GraphicsPass &) = delete;

        GraphicsPass &operator=(const GraphicsPass &) = delete;

        // Get the default render ops for a color buffer.
        static RenderPassBuilder::Attachment::OpsType getDefaultOps() {
            return RenderPassBuilder::Attachment::ColorOps{VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE};
        }

        // Get the default render ops for a stencil buffer.
        static RenderPassBuilder::Attachment::OpsType getStencilOps() {
            return RenderPassBuilder::Attachment::StencilDepthOps{VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                  VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                                  VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                  VK_ATTACHMENT_STORE_OP_DONT_CARE};
        }

        /**
         * Add an image reference that is used in this render pass.
         * @param name the name of the image used
         * @param history the usage history of the image, for tracking purposes
         * @param getter a function to get the location of the image, only if the image is used as a render target.
         * @param ops optional; uses the static defaults if not present.
         * @return the index into the VkAttachmentDescriptions.
         */
        int add(const std::string &name,
                UsageTracker &&history,
                LocationGetter &&getter,
                const std::optional<RenderPassBuilder::Attachment::OpsType> ops = std::nullopt);

        #ifdef fluent
        #  undef fluent
        #endif
        #define fluent GraphicsPass&

        // Specifies that the source image will be resolved to the single destination at the given pass.
        fluent addMultisample(const std::string &source, const std::string &dest, int pass);

        // Build a RenderPassBuilder with the information provided so far.
        std::unique_ptr<RenderPassBuilder> build(int framebuffers);

      private:
        struct AttachmentMeta {
            int index;
            LocationGetter getter;
            vlkx::RenderPassBuilder::Attachment::OpsType ops;
            std::map<int, std::string> multisample;
        };

        void setAttachments();

        void setSubpasses();

        void setDependencies();

        /**
         * Find the first subpass where the given image is used as a render target.
         * @param history the usage history of the image; what it was used at at each subpass.
         * @return nullopt if the image was not used as a render target, the index of the subpass where it was, if not.
         */
        std::optional<int> getFirstRenderTarget(const UsageTracker &history) const;

        /**
         * Return the operations that should be used for the given image attachment.
         * If the user specified ops, it will be checekd against the history.
         * @param name the name of the image to use as the attachment
         * @param history the usage history of the attachment, for internal checks
         * @param userOps operations to use for the image, as an optional override.
         * @return the ColorOps to use for the given attachment.
         */
        RenderPassBuilder::Attachment::OpsType getOps(const std::string &name,
                                                      const UsageTracker &history,
                                                      const std::optional<RenderPassBuilder::Attachment::OpsType> &userOps) const;

        /**
         * Get the usage type of the image.
         * Assumption: an image is only ever used as a color OR depth stencil. Never both.
         * Assumption: Multisample == RenderTarget
         * @param name the name of the image to check
         * @param history the history of the image's usages in the GPU.
         * @return whether the image is a RenderTarget or a DepthStencil buffer.
         */
        ImageUsage::Type getUsageType(const std::string &name, const UsageTracker &history) const;

        /**
         * Ensure that the image is used as type at subpass in its' history.
         */
        bool verifyImageUsage(const UsageTracker &history, int subpass, ImageUsage::Type type) const;

        /**
         * Return whether the subpass is virtual.
         * For a Render Pass, virtual means it is a preprocessing step.
         */
        bool isVirtual(int subpass) const {
            return subpass == getVirtualInitial() || subpass == getVirtualFinal();
        }

        /**
         * Return the subpass index; for virtual passes, it uses an EXTERNAL subpass.
         */
        uint32_t checkSubpass(int subpass) const {
            return isVirtual(subpass) ? VK_SUBPASS_EXTERNAL : (uint32_t) subpass;
        }

        /**
         * Ensure that the image's usages are compatible with a render pass.
         * For example, compute shader linear buffers cannot be used as render targets, etc.
         */
        void verifyHistory(const std::string &image, const UsageTracker &history) const;

        std::map<std::string, AttachmentMeta> metas;
        std::unique_ptr<vlkx::RenderPassBuilder> builder;

    };

    /**
     * The Common Pass implementation for Compute Shaders.
     * That is, shaders that do not write to color buffers.
     * A subpass can execute multiple compute shaders unbarriered, which increases efficiency.
     * We still need to transition images between passes when necessary, hence the wrapper.
     */
    class ComputePass : public CommonPass {
      public:

        ComputePass(const ComputePass &) = delete;

        ComputePass &operator=(const ComputePass &) = delete;

        #ifdef fluent
        #  undef fluent
        #endif
        #define fluent ComputePass&

        /**
         * Add the given image as an attachment to the compute shader pass.
         * @param name the name of the image
         * @param history the usage history of the image
         * @return the ComputePass instance, for chaining.
         */
        fluent add(std::string &&name, UsageTracker &&history);

        fluent add(const std::string &name, UsageTracker &&history) {
            return add(std::string(name), std::move(history));
        }

        /**
         * Run computeOps, insert memory barriers to transition used images into the appropriate format.
         * Images must be a superset of all images that were called with add().
         * Compute_ops must be equal to the number of subpasses.
         * Commands must be recording.
         * @param commands the command buffer to write into.
         * @param queueFamily the family to use for inserting barriers
         * @param images the list of images that were used in the compute pass
         * @param computeOps the compute functions to upload to the GPU
         */
        void execute(const VkCommandBuffer &commands,
                     uint32_t queueFamily,
                     const std::map<std::string, const VkImage *> &images,
                     const std::vector<std::function<void()>> &computeOps) const;

        /**
         * Insert a memory barrier, to transition the layout of the image from the previous to the curent.
         * The barrier is performed using the given queue family.
         * @param commands the command buffer to write into.
         * @param queueFamily the family to use for inserting barriers.
         * @param image the list of images that were used in the compute pass
         * @param prev the previous usage of the image; the state being transitioned from
         * @param current the new usage of the image; the state being transitioned to.
         */
        void barrier(const VkCommandBuffer &commands,
                     uint32_t queueFamily,
                     const VkImage &image,
                     const ImageUsage &prev,
                     const ImageUsage &current) const;

        /**
         * Verify whether the previous usages of the given image in its' history is compatible with a compute shader.
         * For example, a fragment shader output image is not compatible.
         * @param name the name of the image being checked
         * @param history the usage history of the image/
         */
        void verify(const std::string &name, const UsageTracker &history) const;
    };
}