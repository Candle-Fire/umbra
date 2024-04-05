#pragma once

#include <optional>
#include <memory>
#include "GenericRenderPass.h"
#include "vlkx/vulkan/abstraction/ImageUsage.h"
#include "vlkx/vulkan/abstraction/Image.h"

namespace vlkx {

    // A simple and versatile way to configure render passes.
    // Intended to be used with the SimpleRenderPass and the ScreenRenderPass.
    class RendererConfig {
    public:
        RendererConfig(std::vector<std::unique_ptr<vlkx::Image>>& destinations, bool toScreen = true) : renderImages(destinations) {
            numOpaquePasses = 1;
            rendersToScreen = toScreen;
        }

        RendererConfig(int passCount, std::vector<std::unique_ptr<vlkx::Image>>& destinations, bool toScreen = true, std::optional<int> firstTransparent = std::nullopt, std::optional<int> firstOverlay = std::nullopt);

        // Get the number of passes that use the depth buffer.
        int depthPasses() const {
            return numOpaquePasses + numTransparentPasses;
        }

        // Get the total number of passes.
        int passes() const {
            return depthPasses() + numOverlayPasses;
        }

        // Get whether any passes use the depth buffer.
        bool usesDepth() const {
            return depthPasses() > 0;
        }

        // Create the render pass builder. Can be called multiple times.
        void build();

        RendererConfig(RendererConfig&) noexcept = default;
        RendererConfig(const RendererConfig&) = default;

        int numOpaquePasses = 0;
        int numTransparentPasses = 0;
        std::vector<std::unique_ptr<vlkx::Image>>& renderImages;
        bool rendersToScreen;
    private:
        int numOverlayPasses = 0;
    };

    /**
     * Stores all of the information required to use an attachment.
     * This is heavy, so is only used when the image is being finalized.
     */
    struct AttachmentConfig {
        AttachmentConfig(std::string_view name, std::optional<int>* index)
        : name(name), index(*index) {}

        AttachmentConfig& setOps(const RenderPassBuilder::Attachment::OpsType& ops) {
            loadStoreOps = ops;
            return *this;
        }

        AttachmentConfig& setUsage(const ImageUsage& final) {
            finalUsage = final;
            return *this;
        }

        std::string name;
        std::optional<int>& index;
        std::optional<RenderPassBuilder::Attachment::OpsType> loadStoreOps;
        std::optional<ImageUsage> finalUsage;
    };

    /**
     * A lighter version of the Attachment, used primarily in the On-Screen Pass Manager.
     */
    struct Attachment {
        explicit Attachment(std::string_view image) : name(image) {}

        // Adds the image to the tracker and initializes state.
        void add(MultiImageTracker& tracker, const Image& image) {
            tracker.track(name, image.getUsage());
        }

        AttachmentConfig makeConfig() { return { name, &index }; }

        const std::string name;
        std::optional<int> index;
    };

    // Manages Render Passes that will output to the screen.
    // This is necessarily exclusively a graphical pass.
    // If necessary, a depth and stencil buffer will be maintained.
    // The color buffer is automatically assumed to be the swapchain.
    class ScreenRenderPassManager {
    public:
        explicit ScreenRenderPassManager(RendererConfig renderConfig) : config(renderConfig) {}

        ScreenRenderPassManager(const ScreenRenderPassManager&) = delete;
        ScreenRenderPassManager& operator=(const ScreenRenderPassManager&) = delete;

        // Initialize the render pass we're managing.
        void initializeRenderPass();

        std::unique_ptr<vlkx::RenderPass>& getPass() { return pass; }

    private:

        // Prepare the Render Pass builder.
        void preparePassBuilder();

        const RendererConfig config;

        Attachment destinationInfo { "Destination" };
        Attachment multisampleInfo { "Multisample" };
        Attachment depthStencilInfo { "Depth-Stencil" };

        std::unique_ptr<vlkx::Image> depthStencilImage;
        std::unique_ptr<vlkx::RenderPassBuilder> passBuilder;
        std::unique_ptr<vlkx::RenderPass> pass;
    };

    /**
     * A utility namespace used to house a constructor for creating a "simple" render pass with all the defaults.
     */
    namespace SimpleRenderPass {
        static std::unique_ptr<RenderPassBuilder> createBuilder(int framebuffers, const RendererConfig& config, const AttachmentConfig& color, const AttachmentConfig* multisample, const AttachmentConfig* depthStencil, MultiImageTracker& tracker);
    }
}