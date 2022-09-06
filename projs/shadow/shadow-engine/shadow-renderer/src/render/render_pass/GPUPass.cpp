#include "vlkx/render/render_pass/GPUPass.h"

namespace vlkx {

    inline bool needsSynchronization(const ImageUsage& prev, const ImageUsage& curr) {
        if (curr == prev && curr.getAccess() == ImageUsage::Access::ReadOnly)
            return false;
        return true;
    }

    void addUsageToSubpass(const ImageUsage& usage, RenderPassBuilder::SubpassDependency::SubpassMeta* pass) {
        pass->stage |= usage.getStage();
        pass->access |= usage.getAccessFlags();
    }

    void CommonPass::addUsage(std::string &&name, UsageTracker &&tracker) {
        for (const auto& pair : tracker.getUsageMap())
            validate(pair.first, name, false);

        tracker.add(getVirtualInitial(), tracker.getInitialUsage());
        if (tracker.getFinalUsage().has_value())
            tracker.add(getVirtualFinal(), tracker.getFinalUsage().value());

        usageHistory.emplace(std::move(name), std::move(tracker));
    }

    VkImageLayout CommonPass::getInitialLayout(const std::string &name) const {
        return getHistory(name).getUsageMap().begin()->second.getLayout();
    }

    VkImageLayout CommonPass::getFinalLayout(const std::string &name) const {
        return getHistory(name).getUsageMap().rbegin()->second.getLayout();
    }

    VkImageLayout CommonPass::getSubpassLayout(const std::string &name, int subpass) const {
        validate(subpass, name, false);
        return getUsage(name, subpass)->getLayout();
    }

    void CommonPass::update(const std::string &name, MultiImageTracker &tracker) const {
        tracker.update(name, getHistory(name).getUsageMap().rbegin()->second);
    }

    const UsageTracker& CommonPass::getHistory(const std::string &name) const {
        return usageHistory.at(name);
    }

    const ImageUsage* CommonPass::getUsage(const std::string &name, int pass) const {
        validate(pass, name, true);
        const UsageTracker& history = getHistory(name);
        const auto iter = history.getUsageMap().find(pass);
        return iter != history.getUsageMap().end() ? &iter->second : nullptr;
    }

    std::optional<CommonPass::Usages> CommonPass::checkForSync(const std::string &name, int pass) const {
        validate(pass, name, true);
        const UsageTracker& history = getHistory(name);
        const auto currIter = history.getUsageMap().find(pass);
        if (currIter == history.getUsageMap().end())
            return std::nullopt;
        const auto prevIter = std::prev(currIter);

        const ImageUsage& prevUsage = prevIter->second;
        const ImageUsage& currUsage = currIter->second;

        if (!needsSynchronization(prevUsage, currUsage))
            return std::nullopt;

        const int prevSubpass = prevIter->first;
        return CommonPass::Usages { prevSubpass, &prevUsage, &currUsage };
    }

    void CommonPass::validate(int pass, const std::string &image, bool includeVirtual) const {
        if (includeVirtual) {
            if (!(pass >= getVirtualInitial() && pass <= getVirtualFinal()))
                throw std::runtime_error("Subpass out of range.");
        } else {
            if (!(pass >= 0 && pass < numPasses))
                throw std::runtime_error("nv Subpass out of range.");
        }
    }

    int GraphicsPass::add(const std::string &name, UsageTracker &&history, std::function<int(int)> &&getter,
                           const std::optional<RenderPassBuilder::Attachment::OpsType> ops) {
        verifyHistory(name, history);

        const std::optional<int> needsGetter = getFirstRenderTarget(history);
        if (needsGetter.has_value()) {
            if (getter == nullptr)
                throw std::runtime_error("Image " + name + " is used as a render target without a location getter.");
        } else {
            getter = nullptr;
        }

        const int attachmentLocation = static_cast<int>(metas.size());
        metas.insert(
                {
                    name,
                    AttachmentMeta {
                            attachmentLocation,
                        std::move(getter),
                        getOps(name, history, ops),
                        {}
                    },
                }
        );

        addUsage(std::string(name), std::move(history));
        return attachmentLocation;
    }

    GraphicsPass& GraphicsPass::addMultisample(const std::string &source, const std::string &dest, int pass) {
        validate(pass, source, false);

        const auto source_iter = usageHistory.find(source);
        if (source_iter == usageHistory.end())
            throw std::runtime_error("Usage history not found for source image " + source);

        const UsageTracker& source_history = source_iter->second;
        if (!verifyImageUsage(source_history, pass, ImageUsage::Type::RenderTarget))
            throw std::runtime_error("Usage type for source image " + source + " must be render target.");

        const auto dest_iter = usageHistory.find(dest);
        if (dest_iter == usageHistory.end())
            throw std::runtime_error("Usage history not found for destination image " + dest);

        const UsageTracker& dest_history = dest_iter->second;
        if (!verifyImageUsage(dest_history, pass, ImageUsage::Type::Multisample))
            throw std::runtime_error("Usage type for destination image " + dest + " must be multisample");

        auto& targetMap = metas[source].multisample;
        const bool inserted = targetMap.insert( { pass, dest }).second;

        if (!inserted)
            throw std::runtime_error("Image " + source + " is already bound to a multisample.");

        return *this;
    }

    void GraphicsPass::setAttachments() {
        for (const auto& pair : usageHistory) {
            const std::string& name = pair.first;
            const AttachmentMeta& meta = metas[name];
            builder->setAttachment(meta.index, { meta.ops, getInitialLayout(name), getFinalLayout(name) } );
        }
    }

    void GraphicsPass::setSubpasses() {
        for (int pass = 0; pass < numPasses; ++pass) {
            std::vector<RenderPassBuilder::ColorAttachmentMeta> colors;
            std::vector<RenderPassBuilder::ColorAttachmentMeta> multisamples;
            VkAttachmentReference dsRef { VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED };

            // Verify all images used, the long way around.
            for (const auto& pair : usageHistory) {
                const std::string& name = pair.first;
                const ImageUsage* history = getUsage(name, pass);
                if (history == nullptr)
                    continue;

                const AttachmentMeta& meta = metas[name];
                const VkAttachmentReference ref { static_cast<uint32_t>(meta.index), history->getLayout() };

                switch (history->getType()) {
                    // If we're looking at a render target, we need to figure out where it gets its' details from.
                    case ImageUsage::Type::RenderTarget: {
                        const int location = meta.getter(pass);
                        const auto iter = meta.multisample.find(pass);
                        if (iter != meta.multisample.end()) {
                            const std::string& target = iter->second;
                            const ImageUsage* targetUsage = getUsage(target, pass);
                            if (targetUsage == nullptr)
                                throw std::runtime_error("Expected target image to have a usage");
                            multisamples.push_back( { location, metas[target].index, targetUsage->getLayout() });
                        }

                        colors.push_back( { location, static_cast<int>(ref.attachment), ref.layout });

                        break;
                    }

                    case ImageUsage::Type::DepthStencil:
                        if (dsRef.attachment != VK_ATTACHMENT_UNUSED)
                            throw std::runtime_error("Depth stencil used multiple times");
                        dsRef = ref;
                        break;

                    case ImageUsage::Type::Multisample:
                        break;

                    default:
                        throw std::runtime_error("Unreachable?");
                }
            }

            auto colorRefs = RenderPassBuilder::parseColorReferences(colors);
            auto multisampleRefs = RenderPassBuilder::parseMutisampling(colorRefs.size(), multisamples);
            builder->setSubpass(pass, std::move(colorRefs), std::move(multisampleRefs), dsRef);
        }
    }

    void GraphicsPass::setDependencies() {
        if (getVirtualFinal() != numPasses)
            throw std::runtime_error("Virtual subpass mismatch");
        for (int pass = 0; pass <= numPasses; ++pass) {
            std::map<int, RenderPassBuilder::SubpassDependency> deps;

            for (const auto& pair : usageHistory) {
                const auto usage = checkForSync(pair.first, pass);
                if (!usage.has_value()) continue;

                const ImageUsage& prev = usage.value().lastUsage;
                const ImageUsage& curr = usage.value().currentUsage;
                const int sourcePass = usage.value().lastSubpass;

                auto iter = deps.find(sourcePass);
                if (iter == deps.end()) {
                    const RenderPassBuilder::SubpassDependency defaultDep {
                            { checkSubpass(sourcePass), VK_PIPELINE_STAGE_NONE_KHR, VK_ACCESS_NONE_KHR },
                            { checkSubpass(pass), VK_PIPELINE_STAGE_NONE_KHR, VK_ACCESS_NONE_KHR },
                            0
                    };

                    iter = deps.insert( { sourcePass, defaultDep } ).first;
                }

                addUsageToSubpass(prev, &iter->second.source);
                addUsageToSubpass(curr, &iter->second.destination);
            }

            for (const auto& pair : deps) {
                const RenderPassBuilder::SubpassDependency& dep = pair.second;
                builder->addDependency(dep);
            }
        }
    }

    std::unique_ptr<RenderPassBuilder> GraphicsPass::build(int framebuffers) {
        builder = std::make_unique<RenderPassBuilder>();
        builder->setFramebufferCount(framebuffers);

        setAttachments();
        setSubpasses();
        setDependencies();
        return std::move(builder);
    }

    std::optional<int> GraphicsPass::getFirstRenderTarget(const UsageTracker &history) const {
        for (const auto& pair : history.getUsageMap()) {
            const int pass = pair.first;
            if (isVirtual(pass)) continue;

            if (pair.second.getType() == ImageUsage::Type::RenderTarget) return pass;
        }

        return std::nullopt;
    }

    RenderPassBuilder::Attachment::OpsType GraphicsPass::getOps(const std::string &name, const UsageTracker &history,
                                                                 const std::optional<RenderPassBuilder::Attachment::OpsType> &userOps) const {
        const ImageUsage::Type type = getUsageType(name, history);

        if (!userOps.has_value()) {
            switch (type) {
                case ImageUsage::Type::RenderTarget: return getDefaultOps();
                case ImageUsage::Type::DepthStencil: return getStencilOps();
                default:
                    throw std::runtime_error("Unreachable?");
            }
        }

        const RenderPassBuilder::Attachment::OpsType& ops = userOps.value();
        switch (type) {
            case ImageUsage::Type::RenderTarget:
            case ImageUsage::Type::DepthStencil:
                return ops;
            default:
                throw std::runtime_error("Unreachable?");
        }
    }

    ImageUsage::Type GraphicsPass::getUsageType(const std::string &name, const UsageTracker &history) const {
        ImageUsage::Type prev = ImageUsage::Type::DontCare;

        for (const auto& pair : history.getUsageMap()) {
            if (isVirtual(pair.first)) continue;

            ImageUsage::Type type = pair.second.getType();
            if (type == ImageUsage::Type::Multisample)
                type = ImageUsage::Type::RenderTarget;

            if (prev == ImageUsage::Type::DontCare) {
                prev = type;
            } else if (type != prev) {
                throw std::runtime_error("Inconsistent usage type specified for " + name);
            }
        }

        if (prev == ImageUsage::Type::DontCare)
            throw std::runtime_error("Image " + name + " has no usages.");

        return prev;
    }

    bool GraphicsPass::verifyImageUsage(const UsageTracker &history, int subpass, ImageUsage::Type type) const {
        const auto iter = history.getUsageMap().find(subpass);
        return iter != history.getUsageMap().end() && iter->second.getType() == type;
    }

    void GraphicsPass::verifyHistory(const std::string &image, const UsageTracker &history) const {
        for (const auto& pair : history.getUsageMap()) {
            const ImageUsage::Type type = pair.second.getType();
            if (type != ImageUsage::Type::RenderTarget && type != ImageUsage::Type::DepthStencil && type != ImageUsage::Type::Multisample)
                throw std::runtime_error("Invalid usage of " + image + " at subpass " + std::to_string(pair.first));
        }
    }

    ComputePass &ComputePass::add(std::string &&name, UsageTracker &&history) {
        verify(name, history);
        addUsage(std::move(name), std::move(history));
        return *this;
    }

    void ComputePass::execute(const VkCommandBuffer &commands, uint32_t queueFamily,
                              const std::map<std::string, const VkImage*> &images,
                              const std::vector<std::function<void()>>& computeOps) const {

        if (computeOps.size() != numPasses)
            throw std::runtime_error("Compute shader mismatches ops and passes.");

        if (getVirtualFinal() != numPasses)
            throw std::runtime_error("Compute shader attempting to run too many subpasses");

        for (int pass = 0; pass < numPasses; ++pass) {
            for (const auto& pair : usageHistory) {
                const std::string& image = pair.first;
                const auto usage = checkForSync(image, pass);
                if (!usage.has_value()) continue;

                const auto iter = images.find(image);
                if (iter == images.end())
                    throw std::runtime_error("Image " + image + " not provided");

                barrier(commands, queueFamily, *iter->second, usage.value().lastUsage, usage.value().currentUsage);
            }

            if (pass < numPasses)
                computeOps[pass]();
        }
    }

    void ComputePass::barrier(const VkCommandBuffer &commands, uint32_t queueFamily, const VkImage &image,
                              const ImageUsage &prev, const ImageUsage &current) const {
        const VkImageMemoryBarrier barrier {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            prev.getAccessFlags(),
            current.getAccessFlags(),
            prev.getLayout(),
            current.getLayout(),
            queueFamily,
            queueFamily,
            image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            }
        };

        vkCmdPipelineBarrier(
                commands,
                prev.getStage(),
                current.getStage(),
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier
        );
    }

    void ComputePass::verify(const std::string &name, const UsageTracker &history) const {
        for (const auto& pair : history.getUsageMap()) {
            const ImageUsage::Type type = pair.second.getType();
            if (type != ImageUsage::Type::LinearAccess && type != ImageUsage::Type::Sampled && type != ImageUsage::Type::Transfer)
                throw std::runtime_error("Compute shader using an attachment that is not guranteed to be readable.");
        }
    }
}