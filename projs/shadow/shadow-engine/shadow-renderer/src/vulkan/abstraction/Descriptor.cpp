#include "vlkx/vulkan/abstraction/Descriptor.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <map>

using namespace vlkx;
// Returns 'pointer', assuming 'ExpectedType' and 'ActualType' are the same.
template <typename ExpectedType, typename ActualType>
inline const ExpectedType* getPtr(const ActualType* pointer, std::true_type) {
    return pointer;
}

// Returns nullptr, assuming 'ExpectedType' and 'ActualType' are different.
template <typename ExpectedType, typename ActualType>
inline const ExpectedType* getPtr(const ActualType* pointer, std::false_type) {
    return nullptr;
}

template <typename ExpectedType, typename ValueType>
const ExpectedType* getPointer(const std::vector<ValueType>& container) {
    return getPtr<ExpectedType, ValueType>(container.data(), std::is_same<ExpectedType, ValueType>());
}


VkDescriptorPool createPool(std::vector<Descriptor::Meta> metas) {
    std::map<VkDescriptorType, uint32_t> sizes;
    for (const auto& meta : metas) {
        uint32_t length = 0;
        for (const auto& binding : meta.bindings)
            length += binding.length;
        sizes[meta.type] += length;
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto& pair : sizes)
        poolSizes.push_back({ pair.first, pair.second });

    const VkDescriptorPoolCreateInfo create {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr, 0,
        1, static_cast<uint32_t>(poolSizes.size()), poolSizes.data()
    };

    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(VulkanModule::getInstance()->getDevice()->logical, &create, nullptr, &pool) != VK_SUCCESS)
        throw std::runtime_error("Unable to create Descriptor Pool");

    return pool;
}

VkDescriptorSetLayout createLayout(std::vector<Descriptor::Meta> metas, bool dynamic) {
    size_t bindings = 0;
    for (const auto& meta : metas)
        bindings += meta.bindings.size();

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    layoutBindings.reserve(bindings);
    for (const auto& meta : metas)
        for (const auto& binding : meta.bindings)
            layoutBindings.push_back({ binding.bindPoint, meta.type, binding.length, meta.stage, nullptr });

    const VkDescriptorSetLayoutCreateInfo create {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr, static_cast<VkDescriptorSetLayoutCreateFlags>(dynamic ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0),
        static_cast<uint32_t>(layoutBindings.size()), layoutBindings.data()
    };

    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(VulkanModule::getInstance()->getDevice()->logical, &create, nullptr, &layout) != VK_SUCCESS)
        throw std::runtime_error("Unable to create Descriptor Set Layout");

    return layout;
}

VkDescriptorSet allocateSet(const VkDescriptorPool& pool, const VkDescriptorSetLayout& layout) {
    const VkDescriptorSetAllocateInfo allocate {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr, pool, 1, &layout
    };

    VkDescriptorSet set;
    if (vkAllocateDescriptorSets(VulkanModule::getInstance()->getDevice()->logical, &allocate, &set) != VK_SUCCESS)
        throw std::runtime_error("Unable to allocate descriptor set");

    return set;
}

template <typename Type>
std::vector<VkWriteDescriptorSet> createWrites(const VkDescriptorSet& set, VkDescriptorType type, const std::map<uint32_t, std::vector<Type>>& map) {

    std::vector<VkWriteDescriptorSet> sets;
    sets.reserve(map.size());

    for (const auto& pair : map) {
        const auto& info = pair.second;
        sets.push_back(VkWriteDescriptorSet {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            nullptr, set, pair.first, 0,
            static_cast<uint32_t>(info.size()), type,
            getPointer<VkDescriptorImageInfo>(info),
            getPointer<VkDescriptorBufferInfo>(info),
            getPointer<VkBufferView>(info)
        });
    }

    return sets;
}

StaticDescriptor::StaticDescriptor(std::vector<Meta> metas) : Descriptor() {
    pool = createPool(metas);
    const auto layout = createLayout(metas, false);
    setLayout(layout);
    set = allocateSet(pool, layout);
}

const StaticDescriptor& StaticDescriptor::buffers(VkDescriptorType type, const BufferInfos &infos) const {
    return updateSet(createWrites(set, type, infos));
}

const StaticDescriptor& StaticDescriptor::images(VkDescriptorType type, const ImageInfos &infos) const {
    return updateSet(createWrites(set, type, infos));
}

const StaticDescriptor& StaticDescriptor::updateSet(const std::vector<VkWriteDescriptorSet> &write) const {
    vkUpdateDescriptorSets(VulkanModule::getInstance()->getDevice()->logical, write.size(), write.data(), 0, nullptr);
    return *this;
}

void StaticDescriptor::bind(const VkCommandBuffer &commands, const VkPipelineLayout &layout,
                            VkPipelineBindPoint bindPoint) const {
    vkCmdBindDescriptorSets(commands, bindPoint, layout, 0, 1, &set, 0, nullptr);
}

