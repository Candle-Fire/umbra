#include "vlkx/render/render_pass/GenericRenderPass.h"
#include <memory>
#include <string>
#include "vlkx/vulkan/VulkanModule.h"
#include <limits>

namespace vlkx {

    /**
     * Creates the necessary Clear Value struct to erase the given attachment.
     * @param attachment the attachment metadata
     * @return the Clear Value that will erase the attachment's data
     */
    VkClearValue createClearFor(const RenderPassBuilder::Attachment& attachment) {

        VkClearValue clear {};
        if (std::holds_alternative<RenderPassBuilder::Attachment::ColorOps>(attachment.ops))
            clear.color = { { 0, 0, 0, 0 } };
        else {
            clear.depthStencil = { 1.0, 0 };
        }

        return clear;
    }

    /**
     * Convert a RenderPassBuilder attachment to a VkAttachmentDescription.
     * Format will be Undefined.
     * Sample count will be 1.
     */
    VkAttachmentDescription buildAttachment(const RenderPassBuilder::Attachment& attachment) {
        VkAttachmentDescription descriptor {
                {},
                VK_FORMAT_UNDEFINED,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                attachment.layoutInitial,
                attachment.layoutFinal
        };

        if (const auto& ops = std::get_if<RenderPassBuilder::Attachment::ColorOps>(&attachment.ops); ops != nullptr) {
            descriptor.loadOp = ops->LOAD;
            descriptor.storeOp = ops->STORE;
        } else if (const auto& ops = std::get_if<RenderPassBuilder::Attachment::StencilDepthOps>(&attachment.ops); ops != nullptr) {
            descriptor.loadOp = ops->DEPTH_LOAD;
            descriptor.storeOp = ops->DEPTH_STORE;
            descriptor.stencilLoadOp = ops->STENCIL_LOAD;
            descriptor.stencilStoreOp = ops->STENCIL_STORE;
        }

        return descriptor;
    }

    std::vector<VkSubpassDescription> buildSubpassDescriptors(const std::vector<RenderPassBuilder::SubpassAttachments>& attachments) {
        std::vector<VkSubpassDescription> descriptors;
        descriptors.reserve(attachments.size());

        for (const auto& attachment : attachments) {
            descriptors.emplace_back(VkSubpassDescription {
                            {},
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            0,
                            nullptr,
                            static_cast<uint32_t>(attachment.colorReferences.size()),
                            attachment.colorReferences.data(),
                            attachment.multisampleReferences.empty() ? nullptr : attachment.multisampleReferences.data(),
                            attachment.stencilDepthReference.has_value() ? &attachment.stencilDepthReference.value() : nullptr,
                            0,
                            nullptr
                    }
            );
        }

        return descriptors;
    }

    VkSubpassDependency buildSubpassDependency(const RenderPassBuilder::SubpassDependency& dep) {
        return VkSubpassDependency {
            dep.source.index,
            dep.destination.index,
            dep.source.stage,
            dep.destination.stage,
            dep.source.access,
            dep.destination.access,
            dep.flags
        };
    }

    std::vector<int> countColorAttachments(const std::vector<RenderPassBuilder::SubpassAttachments>& attachments) {
        std::vector<int> count;
        count.reserve(attachments.size());

        for (const auto& attachment : attachments)
            count.emplace_back(attachment.colorReferences.size());

        return count;
    }

    std::vector<VkFramebuffer> createFramebuffers(const VkRenderPass& renderPass, const std::vector<std::function<const Image&(int idx)>> getters, int count, const VkExtent2D& extent) {
        std::vector<VkImageView> views(getters.size());

        VkFramebufferCreateInfo framebufferCreate {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            nullptr,
            {},
            renderPass,
            static_cast<uint32_t>(views.size()),
            views.data(),
            extent.width,
            extent.height,
            1
        };

        std::vector<VkFramebuffer> framebuffers(count);
        for (int i = 0; i < framebuffers.size(); ++i) {
            for (int image = 0; image < getters.size(); image++)
                views[image] = getters[image](i).getView();

            vkCreateFramebuffer(VulkanModule::getInstance()->getDevice()->logical, &framebufferCreate, nullptr, &framebuffers[i]);
        }

        return framebuffers;
    }

    std::vector<VkAttachmentReference> RenderPassBuilder::parseColorReferences(std::vector<ColorAttachmentMeta> meta) {
        if (meta.empty())
            return {};

        // Search for the highest location index'd attachment reference.
        // Shaders can define negative locations, so we have to start as low as we can go,
        // and work our way up until we find the highest.
        // If we start at 0, we risk missing an all-negative location set.
        int max = std::numeric_limits<int>::min();
        for (const auto& attachment : meta)
            max = std::max(max, attachment.location);

        std::vector<VkAttachmentReference> references(max + 1, { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
        for (const auto& attachment : meta)
            references[attachment.location] = { static_cast<uint32_t>(attachment.descriptionIdx), attachment.layout };

        return references;
    }

    std::vector<VkAttachmentReference> RenderPassBuilder::parseMutisampling(int colorReferencesCount, std::vector<ColorAttachmentMeta> meta) {
        std::vector<VkAttachmentReference> references(colorReferencesCount, { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED });
        for (const auto& attachment : meta)
            references[attachment.location] = { static_cast<uint32_t>(attachment.descriptionIdx), attachment.layout };

        return references;
    }

    fluent RenderPassBuilder::setFramebufferCount(int count) {
        framebufferCount = count;
        return *this;
    }

    fluent RenderPassBuilder::setAttachment(int idx, const Attachment &attachment) {
        if (idx > clearValues.size())
            clearValues.resize(idx + 1);
        clearValues.at(idx) = createClearFor(attachment);

        if (idx > attachmentDescriptors.size())
            attachmentDescriptors.resize(idx + 1);
        attachmentDescriptors.at(idx) = buildAttachment(attachment);

        if (attachmentDescriptors.size() > attachmentGetters.size())
            attachmentGetters.resize(attachmentDescriptors.size());

        return *this;
    }

    fluent RenderPassBuilder::updateAttachmentBacking(int idx,
                                                      std::function<const Image &(int)> &&getBacking) {
        const Image& img = getBacking(idx);
        attachmentDescriptors[idx].format = img.getFormat();
        attachmentDescriptors[idx].samples = img.getSamples();
        attachmentGetters.at(idx) = std::move(getBacking);
        return *this;
    }

    fluent RenderPassBuilder::setSubpass(int idx, std::vector<VkAttachmentReference> &&color,
                                         std::vector<VkAttachmentReference> &&multisample,
                                         VkAttachmentReference &depthStencil) {
        if (multisample.empty())
            if (multisample.size() != color.size())
                throw std::runtime_error("Constructing a subpass with mismatched color and multisample attachments");

        SubpassAttachments attachments {
            std::move(color), std::move(multisample), depthStencil
        };

        subpassAttachments.emplace_back(attachments);
        return *this;
    }

    fluent RenderPassBuilder::addDependency(const SubpassDependency &dep) {
        subpassDependencies.emplace_back(buildSubpassDependency(dep));
        return *this;
    }

    std::unique_ptr<RenderPass> RenderPassBuilder::build() const {
        if (framebufferCount == 0)
            throw std::runtime_error("No framebuffers in render pass");
        for (int i = 0; i < attachmentGetters.size(); i++)
            if (attachmentGetters[i] == nullptr)
                throw std::runtime_error("Image " + std::to_string(i) + " is not set in render pass");

        const auto descriptors = buildSubpassDescriptors(subpassAttachments);
        const VkRenderPassCreateInfo createInfo {
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            nullptr,
            {},
            static_cast<uint32_t>(attachmentDescriptors.size()),
            attachmentDescriptors.data(),
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data(),
            static_cast<uint32_t>(subpassDependencies.size()),
            subpassDependencies.data()
        };

        VkRenderPass pass;
        if (vkCreateRenderPass(VulkanModule::getInstance()->getDevice()->logical, &createInfo, nullptr, &pass) != VK_SUCCESS)
            throw std::runtime_error("Unable to create render pass");

        const auto framebufferSize = attachmentGetters[0](0).getExtent();

        return std::make_unique<RenderPass> (
                static_cast<int>(descriptors.size()), pass, clearValues, framebufferSize, createFramebuffers(pass, attachmentGetters, framebufferCount.value(), framebufferSize),
                countColorAttachments(subpassAttachments)
        );
    }

    void RenderPass::execute(const VkCommandBuffer &commands, int imageIndex,
                             std::vector<std::function<void(const VkCommandBuffer &)>> ops) const {
        const VkRenderPassBeginInfo begin {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            nullptr,
            renderPass,
            framebuffers[imageIndex],
            {
                    {0, 0},
                    extent
            },
            static_cast<uint32_t>(clearValues.size()),
            clearValues.data()
        };

        vkCmdBeginRenderPass(commands, &begin, VK_SUBPASS_CONTENTS_INLINE);

        for (int i = 0; i < ops.size(); i++) {
            if (i != 0)
                vkCmdNextSubpass(commands, VK_SUBPASS_CONTENTS_INLINE);
            ops[i](commands);
        }

        vkCmdEndRenderPass(commands);
    }

    RenderPass::~RenderPass() {
        for (const auto& fb : framebuffers)
            vkDestroyFramebuffer(VulkanModule::getInstance()->getDevice()->logical, fb, nullptr);
        vkDestroyRenderPass(VulkanModule::getInstance()->getDevice()->logical, renderPass, nullptr);
    }

}