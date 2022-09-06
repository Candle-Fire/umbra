#pragma once

#include <vulkan/vulkan.h>
#include <utility>
#include <vector>
#include <functional>
#include <variant>
#include <optional>
#include "vlkx/vulkan/Tools.h"
#include "vlkx/vulkan/abstraction/Image.h"

namespace vlkx {

    /**
     * Gathers the operations that the GPU should perform when rendering to a framebuffer.
     * Subpasses and dependencies are to be configured automatically using the builder below.
     * RenderPass objects are disposable, and should be discarded when the framebuffer changes.
     */
    class RenderPass {
    public:
        using RenderFunc = std::function<void(const VkCommandBuffer& buffer)>;

        // Delete copy and move constructors to prevent the GPU getting confused with what we're trying to do
        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;
        ~RenderPass();

        RenderPass(int subpasses, VkRenderPass pass, std::vector<VkClearValue> clear, VkExtent2D ext, std::vector<VkFramebuffer> fbs, std::vector<int> attachs)
            : subpassCount(subpasses), renderPass(pass), clearValues(std::move(clear)), extent(ext), framebuffers(std::move(fbs)), attachments(std::move(attachs)) {}

        const VkRenderPass& operator*() const { return renderPass; }

        int getAttachsInSubpass(int subpass) const {
            return attachments[subpass];
        }

        /**
         * Upload all of the subpass render commands to the command buffer.
         * The size of ops must be equal to the number of subpasses in this render pass.
         * @param commands the command buffer to execute on; must be recording
         * @param imageIndex the index of the image on the swapchain that we're rendering to; the target framebuffer.
         * @param ops the render operations to add onto the command buffer.
         */
        void execute(const VkCommandBuffer& commands, int imageIndex, std::vector<RenderFunc> ops) const;

    private:
        // The number of sub-render-passes in this pass.
        const int subpassCount;
        // The VkRenderPass that this class wraps.
        VkRenderPass renderPass;
        // The clear values that will wipe all framebuffers to their empty states.
        const std::vector<VkClearValue> clearValues;
        // The size of the framebuffers (all are the same size)
        const VkExtent2D extent;
        // The framebuffers that we can render to
        const std::vector<VkFramebuffer> framebuffers;
        // The number of color attachments (sampled color images) in each subpass, by subpass index.
        const std::vector<int> attachments;
    };

    /**
     * A stateful, fluent way to create Render Passes.
     * This object can be stored and reused; when the window size changes, simply set the extent and
     *  export a new RenderPass to be used in the pipeline.
     *
     * Allows setting sub-passes, sub-pass dependencies, operations to read and write them, etc.
     */
    class RenderPassBuilder {
    public:

        /**
         * Information required to define an attachment to be used in a render pass.
         * Contains information on the layout, the operations to use on read and write, etc.
         */
        struct Attachment {
            // Operations to use on color attachments.
            struct ColorOps {
                VkAttachmentLoadOp LOAD;            // Load data in the color attachment
                VkAttachmentStoreOp STORE;          // Store data into the color attachment
            };

            // Operations to use on depth and stencil buffers.
            struct StencilDepthOps {
                VkAttachmentLoadOp DEPTH_LOAD;      // Load data in the depth attachment
                VkAttachmentStoreOp DEPTH_STORE;    // Store data in the depth attachment
                VkAttachmentLoadOp STENCIL_LOAD;    // Load data in the stencil attachment
                VkAttachmentStoreOp STENCIL_STORE;  // Store data in the stencil attachment
            };

            using OpsType = std::variant<ColorOps, StencilDepthOps>;

            // The operations that can be performed on this attachment
            OpsType ops;
            // The initial layout of an image in this attachment (pre-GPU upload)
            VkImageLayout layoutInitial;
            // The final layout of an image in this attachment (as seen by the shader)
            VkImageLayout layoutFinal;
        };

        /**
         * Describes attachments used in each subpass, in terms of Vulkan data.
         * Attachment here is used in the conceptual sense, not referring to the Attachment struct above.
         *
         * If multisampleReferences is non-zero, its' size must be equal to colorReferences' size.
         * Each index of multisampleReferences refers to the same-index colorReferences entry.
         *
         * If stencilDepthReference is non-zero, it is shared between all subpasses.
         */
        struct SubpassAttachments {
            std::vector<VkAttachmentReference> colorReferences;
            std::vector<VkAttachmentReference> multisampleReferences;
            std::optional<VkAttachmentReference> stencilDepthReference;
        };

        /**
         * Describes the dependencies between each sub-pass.
         * It describes how each subpass will read or modify the data written by the last subpass, if at all.
         * This dependency information can allow the GPU to run some passes in parallel, and enforce the
         *  strict ordering of those that require it.
         */
        struct SubpassDependency {

            /**
             * Defines some metadata about the subpass.
             * Contains the index of the subpass, how it will use data from the last pass, and what exactly it will do.
             */
            struct SubpassMeta {
                /**
                 * Index of the subpass.
                 */
                uint32_t index;

                /**
                 * Describes how we want to modify the data passed to us from the last subpass.
                 * Will change how the next subpass will wait for the completion of this subpass, if at all.
                 *
                 * VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT:
                 *   read/write to the color attachment
                 *
                 * VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT:
                 * VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT:
                 *   read/write to the depth or stencil attachment
                 *
                 * VK_PIPELINE_STAGE_VERTEX_SHADER_BIT:
                 * VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT:
                 *   read all attachments
                 *
                 */
                VkPipelineStageFlags stage;

                /**
                 * Describes how we want to synchronize with the subpass after this one.
                 *
                 * VK_ACCESS_SHADER_READ_BIT:
                 * VK_ACCESS_SHADER_WRITE_BIT:
                 *   read a texture or write to a color buffer
                 *
                 * VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:
                 * VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
                 * VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
                 * VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
                 *   read/write to an attachment.
                 *
                 * VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:
                 *   for accessing the inputAttachment of a subpass.
                 *
                 * 0:
                 *   do not write, but the next subpass will.
                 *   Will automatically barrier the render pass.
                 */
                VkAccessFlags access;
            };

            SubpassMeta source;         // The source subpass of this dependency (will take effect after this pass completes)
            SubpassMeta destination;    // The destination subpass of this dependency (will take effect before this pass)
            VkDependencyFlags flags;    // Other information that Vulkan needs to know about this dependency; for example, if we use an inputAttachment.
        };

        /**
         * Describes a color attachment.
         * Effectively RGB images that live entirely on the GPU.
         *
         * Describes the resolution mechanics of a multisampled image.
         */
        struct ColorAttachmentMeta {
            int location;               // Where the GPU shaders expect this attachment to be available.
            int descriptionIdx;         // Index of this attachment in the VkAttachmentDescription data.
            VkImageLayout layout;       // Vulkan image layout. Shader optimized or host readable.
        };

        /**
         * Create a list of VkAttachmentReference that describes the attachments used per subpass.
         */
        static std::vector<VkAttachmentReference> parseColorReferences(std::vector<ColorAttachmentMeta> meta);

        /**
         * Create a list of VkAttachmentReference that describes the multisampling setup.
         */
        static std::vector<VkAttachmentReference> parseMutisampling(int colorReferencesCount, std::vector<ColorAttachmentMeta> meta);

        RenderPassBuilder(const RenderPassBuilder&) = delete;
        RenderPassBuilder& operator=(const RenderPassBuilder&) = delete;
        ~RenderPassBuilder() = default;
        RenderPassBuilder() = default;

        /** Fluent API Features; chain calls to set data on the render pass.*/
        #define fluent RenderPassBuilder&

        // Set the number of framebuffers in the render pass
        fluent setFramebufferCount(int count);
        // Set an attachment description in the render pass
        fluent setAttachment(int idx, const Attachment& attachment);
        // Update the image backing an attachment. The function must be executable during execute() later on.
        fluent updateAttachmentBacking(int idx, std::function<const Image&(int idx)>&& getBacking);
        // Set a specific subpass. Use the static parse methods to create these vectors.
        fluent setSubpass(int idx, std::vector<VkAttachmentReference>&& color, std::vector<VkAttachmentReference>&& multisample, VkAttachmentReference& depthStencil);
        // Add a dependency between two subpasses.
        fluent addDependency(const SubpassDependency& dep);

        // Build the Render Pass with all the information given.
        // Can be called multiple times with the same Builder.
        [[nodiscard]] std::unique_ptr<vlkx::RenderPass> build() const;

    private:
        // Number of framebuffers in the render pass
        std::optional<int> framebufferCount;
        // Descriptions of used attachments
        std::vector<VkAttachmentDescription> attachmentDescriptors;
        // Functions to return attachment images.
        std::vector<std::function<const Image&(int idx)>> attachmentGetters;
        // Values to clear all attachments
        std::vector<VkClearValue> clearValues;
        // Descriptions of subpasses.
        std::vector<SubpassAttachments> subpassAttachments;
        // Descriptions of subpass dependencies.
        std::vector<VkSubpassDependency> subpassDependencies;
    };
}