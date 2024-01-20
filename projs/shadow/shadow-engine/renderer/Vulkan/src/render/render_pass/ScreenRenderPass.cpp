#include "shadow/renderer/vulkan/vlkx/render/render_pass/ScreenRenderPass.h"
#include "shadow/renderer/vulkan/vlkx/vulkan/abstraction/Image.h"
#include "shadow/renderer/vulkan/vlkx/render/render_pass/GPUPass.h"
#include "shadow/renderer/vulkan/vlkx/vulkan/VulkanModule.h"

namespace vlkx {

    void checkSubpass(int pass, int high) { if (pass < 1 || pass > high) throw std::runtime_error("Subpass index too high"); }

    void addAttachment(const AttachmentConfig& config, GraphicsPass& pass, MultiImageTracker& tracker, GraphicsPass::LocationGetter&& getter, const std::function<void(UsageTracker&)>& populateHistory) {
        const std::string& name = config.name;
        if (!tracker.isTracking(name))
            throw std::runtime_error("Attempting to add image " + name + " that is not tracked");

        UsageTracker history { tracker.get(name) };
        populateHistory(history);

        if (config.finalUsage.has_value())
            history.setFinal(config.finalUsage.value());

        config.index = pass.add(name, std::move(history), std::move(getter), config.loadStoreOps);
        pass.update(name, tracker);
    }

    RendererConfig::RendererConfig(int passCount, std::vector<std::unique_ptr<vlkx::Image>>& destinations, bool toScreen, std::optional<int> firstTransparent, std::optional<int> firstOverlay) : renderImages(destinations) {
        if (passCount < 1)
            throw std::runtime_error("Creating a RendererConfig with less than 1 subpass.");

        if (firstTransparent.has_value())
            checkSubpass(firstTransparent.value(), passCount);
        if (firstOverlay.has_value())
            checkSubpass(firstOverlay.value(), passCount);

        if (firstOverlay.has_value())
            numOverlayPasses = passCount - firstOverlay.value();
        if (firstTransparent.has_value()) {
            numOpaquePasses = firstTransparent.value();
            numTransparentPasses = passCount - numOpaquePasses - numOverlayPasses;
        } else {
            numOpaquePasses = passCount - numOverlayPasses;
        }

        rendersToScreen = toScreen;
    }

    std::unique_ptr<RenderPassBuilder> SimpleRenderPass::createBuilder(int framebuffers,
                                                                       const vlkx::RendererConfig &config,
                                                                       const vlkx::AttachmentConfig &color,
                                                                       const vlkx::AttachmentConfig *multisample,
                                                                       const vlkx::AttachmentConfig *depthStencil,
                                                                       vlkx::MultiImageTracker &tracker) {
        const int passes = config.passes();
        const int firstPass = 0;
        const int lastPass = passes - 1;

        const auto getLocation = [](int pass) { return 0; };
        bool usesMultisample = multisample != nullptr;
        bool usesDepth = depthStencil != nullptr;

        const bool passesUsingDepth = config.depthPasses() > 0;

        if (usesDepth) {
            if (passesUsingDepth == 0)
                throw std::runtime_error("Depth stencil defined, but never used.");
        } else
            if (passesUsingDepth > 0)
                throw std::runtime_error("Depth stencil used, but never defined.");

        GraphicsPass graphicsPass(passes);

        addAttachment(color, graphicsPass, tracker, getLocation, [&](UsageTracker& history) {
            if (usesMultisample)
                history.add(lastPass, ImageUsage::multisample());
            else
                history.add(firstPass, lastPass, ImageUsage::renderTarget(0));
        });

        if (usesMultisample) {
            addAttachment(*multisample, graphicsPass, tracker, getLocation, [&](UsageTracker& history) {
                history.add(firstPass, lastPass, ImageUsage::renderTarget(0));
            });
            graphicsPass.addMultisample(multisample->name, color.name, lastPass);
        }

        if (usesDepth)
            addAttachment(*depthStencil, graphicsPass, tracker, getLocation, [&](UsageTracker& history) {
                if (config.numOpaquePasses > 0) {
                    const int lastOpaque = config.numOpaquePasses - 1;
                    history.add(firstPass, lastOpaque, ImageUsage::depthStencil(ImageUsage::Access::ReadWrite));
                }

                if (config.numTransparentPasses > 0) {
                    const int firstTransparent = config.numOpaquePasses;
                    const int lastTransparent = firstTransparent + config.numTransparentPasses - 1;
                    history.add(firstTransparent, lastTransparent, ImageUsage::depthStencil(ImageUsage::Access::ReadOnly));
                }
            });

        return graphicsPass.build(framebuffers);

    }

    void ScreenRenderPassManager::initializeRenderPass() {
        if (config.usesDepth()) {
            depthStencilImage = MultisampleImage::createDepthStencil(VulkanModule::getInstance()->getSwapchain()->extent, std::nullopt);
        }

        if (passBuilder == nullptr)
            preparePassBuilder();

        passBuilder->updateAttachmentBacking(destinationInfo.index.value(), [this](int index) -> const Image& {
            return *config.renderImages[index];
        });

        if (depthStencilImage != nullptr)
            passBuilder->updateAttachmentBacking(depthStencilInfo.index.value(), [this](int index) -> const Image& {
                return *depthStencilImage;
            });

        pass = passBuilder->build();
    }

    void ScreenRenderPassManager::preparePassBuilder() {
        const bool usesDepth = depthStencilImage != nullptr;
        const bool usesMultisampling = false;

        MultiImageTracker tracker;
        destinationInfo.add(tracker, *config.renderImages[0]);
        if (usesDepth)
            depthStencilInfo.add(tracker, *depthStencilImage);
        if (usesMultisampling)
            // TODO
            (void) 0;

        auto colorConfig = destinationInfo.makeConfig();
        if (config.rendersToScreen)
            colorConfig.setUsage(ImageUsage::presentation());
        else
            colorConfig.setUsage(ImageUsage::sampledFragment());
        const auto multisampleConfig = multisampleInfo.makeConfig();
        const auto depthConfig = depthStencilInfo.makeConfig();

        passBuilder = SimpleRenderPass::createBuilder(config.renderImages.size(), config, colorConfig, usesMultisampling ? &multisampleConfig : nullptr, usesDepth ? &depthConfig : nullptr, tracker);
    }
}
