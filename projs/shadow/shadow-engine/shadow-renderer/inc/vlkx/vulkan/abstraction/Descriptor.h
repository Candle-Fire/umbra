#pragma once

#include <map>
#include <vector>
#include <vulkan/vulkan.h>
#include <temp/model/Loader.h>
#include <vlkx/vulkan/VulkanModule.h>

namespace vlkx {

    class Descriptor {
    public:
        using TextureType = vlkxtemp::ModelLoader::TextureType;
        using BufferInfos = std::map<uint32_t, std::vector<VkDescriptorBufferInfo>>;
        using ImageInfos = std::map<uint32_t, std::vector<VkDescriptorImageInfo>>;

        struct Meta {
            struct Binding {
                uint32_t bindPoint;
                uint32_t length;
            };

            VkDescriptorType type;
            VkShaderStageFlags stage;
            std::vector<Binding> bindings;
        };

        Descriptor(const Descriptor&) = delete;
        Descriptor& operator=(const Descriptor&) = delete;

        virtual ~Descriptor() {
            vkDestroyDescriptorSetLayout(VulkanModule::getInstance()->getDevice()->logical, layout, nullptr);
        }

        const VkDescriptorSetLayout& getLayout() const { return layout; }

    protected:
        explicit Descriptor() = default;

        void setLayout(const VkDescriptorSetLayout& newLayout) { layout = newLayout; }

    private:
        VkDescriptorSetLayout layout;
    };

    class StaticDescriptor : public Descriptor {
    public:

        StaticDescriptor(std::vector<Meta> metas);
        StaticDescriptor(const StaticDescriptor&) = delete;
        StaticDescriptor& operator=(const StaticDescriptor&) = delete;

        ~StaticDescriptor() override {
            vkDestroyDescriptorPool(VulkanModule::getInstance()->getDevice()->logical, pool, nullptr);
        }

        const StaticDescriptor& buffers(VkDescriptorType type, const BufferInfos& infos) const;
        const StaticDescriptor& images(VkDescriptorType type, const ImageInfos& infos) const;

        void bind(const VkCommandBuffer& commands, const VkPipelineLayout& layout, VkPipelineBindPoint bindPoint) const;

    private:

        const StaticDescriptor& updateSet(const std::vector<VkWriteDescriptorSet>& write) const;

        VkDescriptorPool pool;
        VkDescriptorSet set;
    };

    // TODO: dynamic sets
}