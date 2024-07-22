#include <algorithm>
#include <cfloat>
#include <renderer/interfaces/VulkanInterface.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <unordered_set>
#include <shadow/assets/fs/file.h>
#include <shadow/core/Time.h>

#include "spdlog/spdlog.h"

namespace rx {
    namespace vulkan {
        enum BindShift {
            BUFFER = 0,
            TEXTURE = 1000,
            UNIFORM = 2000,
            SAMPLER = 3000,
          };

        /**
         * @brief Functions to convert from abstract data types to Vulkan specific enum values
         */
        namespace convert {

            // Converters:
            constexpr VkFormat Format(ImageFormat value) {
                switch (value) {
                    case ImageFormat::UNKNOWN:
                        return VK_FORMAT_UNDEFINED;
                    case ImageFormat::R32G32B32A32_FLOAT:
                        return VK_FORMAT_R32G32B32A32_SFLOAT;
                    case ImageFormat::R32G32B32A32_UINT:
                        return VK_FORMAT_R32G32B32A32_UINT;
                    case ImageFormat::R32G32B32A32_SINT:
                        return VK_FORMAT_R32G32B32A32_SINT;
                    case ImageFormat::R32G32B32_FLOAT:
                        return VK_FORMAT_R32G32B32_SFLOAT;
                    case ImageFormat::R32G32B32_UINT:
                        return VK_FORMAT_R32G32B32_UINT;
                    case ImageFormat::R32G32B32_SINT:
                        return VK_FORMAT_R32G32B32_SINT;
                    case ImageFormat::R16G16B16A16_FLOAT:
                        return VK_FORMAT_R16G16B16A16_SFLOAT;
                    case ImageFormat::R16G16B16A16_UNORM:
                        return VK_FORMAT_R16G16B16A16_UNORM;
                    case ImageFormat::R16G16B16A16_UINT:
                        return VK_FORMAT_R16G16B16A16_UINT;
                    case ImageFormat::R16G16B16A16_SNORM:
                        return VK_FORMAT_R16G16B16A16_SNORM;
                    case ImageFormat::R16G16B16A16_SINT:
                        return VK_FORMAT_R16G16B16A16_SINT;
                    case ImageFormat::R32G32_FLOAT:
                        return VK_FORMAT_R32G32_SFLOAT;
                    case ImageFormat::R32G32_UINT:
                        return VK_FORMAT_R32G32_UINT;
                    case ImageFormat::R32G32_SINT:
                        return VK_FORMAT_R32G32_SINT;
                    case ImageFormat::D32_FLOAT_S8X24_UINT:
                        return VK_FORMAT_D32_SFLOAT_S8_UINT;
                    case ImageFormat::R10G10B10A2_UNORM:
                        return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
                    case ImageFormat::R10G10B10A2_UINT:
                        return VK_FORMAT_A2B10G10R10_UINT_PACK32;
                    case ImageFormat::R11G11B10_FLOAT:
                        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                    case ImageFormat::R8G8B8A8_UNORM:
                        return VK_FORMAT_R8G8B8A8_UNORM;
                    case ImageFormat::R8G8B8A8_UNORM_SRGB:
                        return VK_FORMAT_R8G8B8A8_SRGB;
                    case ImageFormat::R8G8B8A8_UINT:
                        return VK_FORMAT_R8G8B8A8_UINT;
                    case ImageFormat::R8G8B8A8_SNORM:
                        return VK_FORMAT_R8G8B8A8_SNORM;
                    case ImageFormat::R8G8B8A8_SINT:
                        return VK_FORMAT_R8G8B8A8_SINT;
                    case ImageFormat::R16G16_FLOAT:
                        return VK_FORMAT_R16G16_SFLOAT;
                    case ImageFormat::R16G16_UNORM:
                        return VK_FORMAT_R16G16_UNORM;
                    case ImageFormat::R16G16_UINT:
                        return VK_FORMAT_R16G16_UINT;
                    case ImageFormat::R16G16_SNORM:
                        return VK_FORMAT_R16G16_SNORM;
                    case ImageFormat::R16G16_SINT:
                        return VK_FORMAT_R16G16_SINT;
                    case ImageFormat::D32_FLOAT:
                        return VK_FORMAT_D32_SFLOAT;
                    case ImageFormat::R32_FLOAT:
                        return VK_FORMAT_R32_SFLOAT;
                    case ImageFormat::R32_UINT:
                        return VK_FORMAT_R32_UINT;
                    case ImageFormat::R32_SINT:
                        return VK_FORMAT_R32_SINT;
                    case ImageFormat::D24_UNORM_S8_UINT:
                        return VK_FORMAT_D24_UNORM_S8_UINT;
                    case ImageFormat::R9G9B9E5_SHARED:
                        return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
                    case ImageFormat::R8G8_UNORM:
                        return VK_FORMAT_R8G8_UNORM;
                    case ImageFormat::R8G8_UINT:
                        return VK_FORMAT_R8G8_UINT;
                    case ImageFormat::R8G8_SNORM:
                        return VK_FORMAT_R8G8_SNORM;
                    case ImageFormat::R8G8_SINT:
                        return VK_FORMAT_R8G8_SINT;
                    case ImageFormat::R16_FLOAT:
                        return VK_FORMAT_R16_SFLOAT;
                    case ImageFormat::D16_UNORM:
                        return VK_FORMAT_D16_UNORM;
                    case ImageFormat::R16_UNORM:
                        return VK_FORMAT_R16_UNORM;
                    case ImageFormat::R16_UINT:
                        return VK_FORMAT_R16_UINT;
                    case ImageFormat::R16_SNORM:
                        return VK_FORMAT_R16_SNORM;
                    case ImageFormat::R16_SINT:
                        return VK_FORMAT_R16_SINT;
                    case ImageFormat::R8_UNORM:
                        return VK_FORMAT_R8_UNORM;
                    case ImageFormat::R8_UINT:
                        return VK_FORMAT_R8_UINT;
                    case ImageFormat::R8_SNORM:
                        return VK_FORMAT_R8_SNORM;
                    case ImageFormat::R8_SINT:
                        return VK_FORMAT_R8_SINT;
                    case ImageFormat::BC1_UNORM:
                        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
                    case ImageFormat::BC1_UNORM_SRGB:
                        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
                    case ImageFormat::BC2_UNORM:
                        return VK_FORMAT_BC2_UNORM_BLOCK;
                    case ImageFormat::BC2_UNORM_SRGB:
                        return VK_FORMAT_BC2_SRGB_BLOCK;
                    case ImageFormat::BC3_UNORM:
                        return VK_FORMAT_BC3_UNORM_BLOCK;
                    case ImageFormat::BC3_UNORM_SRGB:
                        return VK_FORMAT_BC3_SRGB_BLOCK;
                    case ImageFormat::BC4_UNORM:
                        return VK_FORMAT_BC4_UNORM_BLOCK;
                    case ImageFormat::BC4_SNORM:
                        return VK_FORMAT_BC4_SNORM_BLOCK;
                    case ImageFormat::BC5_UNORM:
                        return VK_FORMAT_BC5_UNORM_BLOCK;
                    case ImageFormat::BC5_SNORM:
                        return VK_FORMAT_BC5_SNORM_BLOCK;
                    case ImageFormat::B8G8R8A8_UNORM:
                        return VK_FORMAT_B8G8R8A8_UNORM;
                    case ImageFormat::B8G8R8A8_UNORM_SRGB:
                        return VK_FORMAT_B8G8R8A8_SRGB;
                    case ImageFormat::BC6H_UF16:
                        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
                    case ImageFormat::BC6H_SF16:
                        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
                    case ImageFormat::BC7_UNORM:
                        return VK_FORMAT_BC7_UNORM_BLOCK;
                    case ImageFormat::BC7_UNORM_SRGB:
                        return VK_FORMAT_BC7_SRGB_BLOCK;
                    case ImageFormat::NV12:
                        return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
                }
                return VK_FORMAT_UNDEFINED;
            }

            constexpr VkCompareOp Comparison(ComparisonFunc value) {
                switch (value) {
                    case ComparisonFunc::NEVER:
                        return VK_COMPARE_OP_NEVER;
                    case ComparisonFunc::LESS:
                        return VK_COMPARE_OP_LESS;
                    case ComparisonFunc::EQUAL:
                        return VK_COMPARE_OP_EQUAL;
                    case ComparisonFunc::LESS_EQUAL:
                        return VK_COMPARE_OP_LESS_OR_EQUAL;
                    case ComparisonFunc::GREATER:
                        return VK_COMPARE_OP_GREATER;
                    case ComparisonFunc::NOT_EQUAL:
                        return VK_COMPARE_OP_NOT_EQUAL;
                    case ComparisonFunc::GREATER_EQUAL:
                        return VK_COMPARE_OP_GREATER_OR_EQUAL;
                    case ComparisonFunc::ALWAYS:
                        return VK_COMPARE_OP_ALWAYS;
                    default:
                        return VK_COMPARE_OP_NEVER;
                }
            }

            constexpr VkBlendFactor BlendSource(BlendData value) {
                switch (value) {
                    case BlendData::CLEAR:
                        return VK_BLEND_FACTOR_ZERO;
                    case BlendData::ONE:
                        return VK_BLEND_FACTOR_ONE;
                    case BlendData::SOURCE_COLOR:
                        return VK_BLEND_FACTOR_SRC_COLOR;
                    case BlendData::SOURCE_COLOR_INVERSE:
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                    case BlendData::SOURCE_ALPHA:
                        return VK_BLEND_FACTOR_SRC_ALPHA;
                    case BlendData::SOURCE_ALPHA_INVERSE:
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                    case BlendData::DESTINATION_ALPHA:
                        return VK_BLEND_FACTOR_DST_ALPHA;
                    case BlendData::DESTINATION_ALPHA_INVERSE:
                        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                    case BlendData::DESTINATION_COLOR:
                        return VK_BLEND_FACTOR_DST_COLOR;
                    case BlendData::DESTINATION_COLOR_INVERSE:
                        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                    case BlendData::SOURCE_ALPHA_SATURATION:
                        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
                    case BlendData::BLENDING_FACTOR:
                        return VK_BLEND_FACTOR_CONSTANT_COLOR;
                    case BlendData::BLENDING_FACTOR_ONE_MINUS:
                        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
                    case BlendData::SOURCE1_COLOR:
                        return VK_BLEND_FACTOR_SRC1_COLOR;
                    case BlendData::SOURCE1_COLOR_INVERSE:
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
                    case BlendData::SOURCE1_ALPHA:
                        return VK_BLEND_FACTOR_SRC1_ALPHA;
                    case BlendData::SOURCE1_ALPHA_INVERSE:
                        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
                    default:
                        return VK_BLEND_FACTOR_ZERO;
                }
            }

            constexpr VkBlendOp BlendOperation(BlendOp value)  {
                switch (value) {
                    case BlendOp::ADD:
                        return VK_BLEND_OP_ADD;
                    case BlendOp::SUBTRACT:
                        return VK_BLEND_OP_SUBTRACT;
                    case BlendOp::SUBTRACT_INVERSE:
                        return VK_BLEND_OP_REVERSE_SUBTRACT;
                    case BlendOp::MINIMAL:
                        return VK_BLEND_OP_MIN;
                    case BlendOp::MAXIMAL:
                        return VK_BLEND_OP_MAX;
                    default:
                        return VK_BLEND_OP_ADD;
                }
            }

            constexpr VkSamplerAddressMode TileMode(ImageTiling value, const VkPhysicalDeviceVulkan12Features& feats) {
                switch (value) {
                    case ImageTiling::WRAP:
                        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
                    case ImageTiling::MIRROR:
                        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                    case ImageTiling::CLAMP:
                        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                    case ImageTiling::BORDER:
                        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                    case ImageTiling::MIRROR_ONCE:
                        if (feats.samplerMirrorClampToEdge == VK_TRUE)
                            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

                    return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                    default:
                        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                }
            }

            constexpr VkBorderColor SamplerBorder(SamplerBorderColor value) {
                switch (value) {
                    case SamplerBorderColor::TRANSPARENT_BLACK:
                        return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
                    case SamplerBorderColor::OPAQUE_BLACK:
                        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
                    case SamplerBorderColor::OPAQUE_WHITE:
                        return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                    default:
                        return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
                }
            }

            constexpr VkStencilOp StencilOperation(StencilOp value) {
                switch (value) {
                    case StencilOp::KEEP:
                        return VK_STENCIL_OP_KEEP;
                    case StencilOp::DISCARD:
                        return VK_STENCIL_OP_ZERO;
                    case StencilOp::REPLACE:
                        return VK_STENCIL_OP_REPLACE;
                    case StencilOp::INCREASE_SATURATION:
                        return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                    case StencilOp::DECREASE_SATURATION:
                        return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                    case StencilOp::INVERT:
                        return VK_STENCIL_OP_INVERT;
                    case StencilOp::INCR:
                        return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                    case StencilOp::DECR:
                        return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                    default:
                        return VK_STENCIL_OP_KEEP;
                }
            }

            constexpr VkImageLayout ImageLayout(ResourceState value) {
                switch (value) {
                    case ResourceState::UNDEFINED:
                        return VK_IMAGE_LAYOUT_UNDEFINED;
                    case ResourceState::RENDER_TARGET:
                        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    case ResourceState::DEPTH_STENCIL:
                        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    case ResourceState::DEPTH_STENCIL_RO:
                        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                    case ResourceState::SHADER_RESOURCE:
                    case ResourceState::SHADER_RESOURCE_COMPUTE:
                        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    case ResourceState::UNORDERED_ACCESS:
                    case ResourceState::COPY_SOURCE:
                    case ResourceState::COPY_DESTINATION:
                        return VK_IMAGE_LAYOUT_GENERAL;
                    case ResourceState::SHADING_RATE_SOURCE:
                        return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
                    case ResourceState::VIDEO_DECODE_SOURCE:
                    case ResourceState::VIDEO_DECODE_DESTINATION:
                        return VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR;
                    default:
                        return VK_IMAGE_LAYOUT_UNDEFINED;
                }
            }

            constexpr VkShaderStageFlags ShaderStages(ShaderStage value) {
                switch (value) {
                    case ShaderStage::MS:
                        return VK_SHADER_STAGE_MESH_BIT_EXT;
                    case ShaderStage::AS:
                        return VK_SHADER_STAGE_TASK_BIT_EXT;
                    case ShaderStage::VS:
                        return VK_SHADER_STAGE_VERTEX_BIT;
                    case ShaderStage::HS:
                        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    case ShaderStage::DS:
                        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    case ShaderStage::GS:
                        return VK_SHADER_STAGE_GEOMETRY_BIT;
                    case ShaderStage::FS:
                        return VK_SHADER_STAGE_FRAGMENT_BIT;
                    case ShaderStage::CS:
                        return VK_SHADER_STAGE_COMPUTE_BIT;
                    default:
                        return VK_SHADER_STAGE_ALL;
                }
            }

            constexpr VkImageAspectFlags ImageAspects(ImageAspect value) {
                switch (value) {
                    default:
                    case ImageAspect::COLOR:
                        return VK_IMAGE_ASPECT_COLOR_BIT;
                    case ImageAspect::DEPTH:
                        return VK_IMAGE_ASPECT_DEPTH_BIT;
                    case ImageAspect::STENCIL:
                        return VK_IMAGE_ASPECT_STENCIL_BIT;
                    case ImageAspect::LUMINANCE:
                        return VK_IMAGE_ASPECT_PLANE_0_BIT;
                    case ImageAspect::CHROMINANCE:
                        return VK_IMAGE_ASPECT_PLANE_1_BIT;
                }
            }

            constexpr VkPipelineStageFlags2 PipelineStage(ResourceState value) {
                VkPipelineStageFlags2 flags = VK_PIPELINE_STAGE_2_NONE;

                if (has_flag(value, ResourceState::SHADER_RESOURCE) ||
                    has_flag(value, ResourceState::SHADER_RESOURCE_COMPUTE) ||
                    has_flag(value, ResourceState::UNORDERED_ACCESS) ||
                    has_flag(value, ResourceState::CONSTANT_BUFFER))
                    flags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

                if (has_flag(value, ResourceState::COPY_SOURCE) ||
                    has_flag(value, ResourceState::COPY_DESTINATION)) {
                    flags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                    }

                if (has_flag(value, ResourceState::RENDER_TARGET))
                    flags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                if (has_flag(value, ResourceState::DEPTH_STENCIL) || has_flag(value, ResourceState::DEPTH_STENCIL_RO))
                    flags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                if (has_flag(value, ResourceState::SHADING_RATE_SOURCE))
                    flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
                if (has_flag(value, ResourceState::VERTEX_BUFFER))
                    flags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
                if (has_flag(value, ResourceState::INDEX_BUFFER))
                    flags |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
                if (has_flag(value, ResourceState::INDIRECT_ARGUMENT))
                    flags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                if (has_flag(value, ResourceState::RT_ACCELERATION))
                    flags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
                if (has_flag(value, ResourceState::PREDICATION))
                    flags |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT;
                if (has_flag(value, ResourceState::VIDEO_DECODE_DESTINATION) || has_flag(value, ResourceState::VIDEO_DECODE_SOURCE))
                    flags |= VK_PIPELINE_STAGE_2_VIDEO_DECODE_BIT_KHR;
                return flags;
            }

            constexpr VkAccessFlags2 ResourceAccess(ResourceState value) {
                VkAccessFlags2 flags = 0;

                if (has_flag(value, ResourceState::SHADER_RESOURCE))
                    flags |= VK_ACCESS_2_SHADER_READ_BIT;
                if (has_flag(value, ResourceState::SHADER_RESOURCE_COMPUTE))
                    flags |= VK_ACCESS_2_SHADER_READ_BIT;
                if (has_flag(value, ResourceState::UNORDERED_ACCESS))
                    flags |= VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                if (has_flag(value, ResourceState::COPY_SOURCE))
                    flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
                if (has_flag(value, ResourceState::COPY_DESTINATION))
                    flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
                if (has_flag(value, ResourceState::RENDER_TARGET))
                    flags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT| VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                if (has_flag(value, ResourceState::DEPTH_STENCIL))
                    flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT| VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                if (has_flag(value, ResourceState::DEPTH_STENCIL_RO))
                    flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                if (has_flag(value, ResourceState::VERTEX_BUFFER))
                    flags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
                if (has_flag(value, ResourceState::INDEX_BUFFER))
                    flags |= VK_ACCESS_2_INDEX_READ_BIT;
                if (has_flag(value, ResourceState::CONSTANT_BUFFER))
                    flags |= VK_ACCESS_2_UNIFORM_READ_BIT;
                if (has_flag(value, ResourceState::INDIRECT_ARGUMENT))
                    flags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
                if (has_flag(value, ResourceState::PREDICATION))
                    flags |= VK_ACCESS_2_CONDITIONAL_RENDERING_READ_BIT_EXT;
                if (has_flag(value, ResourceState::SHADING_RATE_SOURCE))
                    flags |= VK_ACCESS_2_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR;
                if (has_flag(value, ResourceState::VIDEO_DECODE_DESTINATION))
                    flags |= VK_ACCESS_2_VIDEO_DECODE_WRITE_BIT_KHR;
                if (has_flag(value, ResourceState::VIDEO_DECODE_SOURCE))
                    flags |= VK_ACCESS_2_VIDEO_DECODE_READ_BIT_KHR;

                return flags;
            }

            constexpr VkComponentSwizzle ComponentSwizzling(ComponentSwizzle value) {
                switch (value) {
                    default:
                        return VK_COMPONENT_SWIZZLE_IDENTITY;
                    case ComponentSwizzle::R:
                        return VK_COMPONENT_SWIZZLE_R;
                    case ComponentSwizzle::G:
                        return VK_COMPONENT_SWIZZLE_G;
                    case ComponentSwizzle::B:
                        return VK_COMPONENT_SWIZZLE_B;
                    case ComponentSwizzle::A:
                        return VK_COMPONENT_SWIZZLE_A;
                    case ComponentSwizzle::ZERO:
                        return VK_COMPONENT_SWIZZLE_ZERO;
                    case ComponentSwizzle::ONE:
                        return VK_COMPONENT_SWIZZLE_ONE;
                }
            }

            constexpr VkComponentMapping Swizzling(Swizzle value) {
                return {
                    .r = ComponentSwizzling(value.r),
                    .g = ComponentSwizzling(value.g),
                    .b = ComponentSwizzling(value.b),
                    .a = ComponentSwizzling(value.a)
                };
            }
        } // /convert

        /**
         * @brief Verify that an extension with the given name is present / available
         * @param ext the name of a desired extension
         * @param available the list of extensions supported by the driver/hardware
         * @return
         */
        bool CheckExtension(const char* ext, const std::vector<VkExtensionProperties>& available) {
            for (const auto& x : available)
                if (strcmp(x.extensionName, ext) == 0)
                    return true;
            return false;
        }

        /**
         * @brief Verify that all required layers are supported by the hardware and driver.
         * @param requiredLayers the list of layers that the program requires
         * @param available the list of layers that the driver/hardware can provide
         * @return
         */
        bool ValidateLayers(const std::vector<const char*>& requiredLayers, const std::vector<VkLayerProperties>& available) {
            for (auto layer : requiredLayers) {
                bool found = false;
                for (auto& avail : available) {
                    if (strcmp(avail.layerName, layer) == 0) {
                        found = true;
                        break;
                    }
                }

                if (!found) return false;
            }

            return true;
        }

        /**
         * @brief Wrap Vulkan messages into the engine's logging system.
         */
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* callback, void* user) {
            if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
                spdlog::warn(std::string("[Vulkan Warning] ") + callback->pMessage);
            else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                spdlog::error(std::string("[Vulkan Error] ") + callback->pMessage);
            return VK_FALSE;
        }

        /**
         * @brief Vulkan-Specific specializations over abstract types, containing Vk types. Accessed via `->internal.get()` on the abstract types.
         */
        namespace structs {

            // A VkDescriptorSetLayoutBinding which tracks whether or not it is currently being used.
            struct TrackedBinding {
                VkDescriptorSetLayoutBinding binding = {};
                bool used = false;
            };

            // A Buffer subtype that holds all the Vulkan-specific information for a normal Buffer.
            struct VulkanBuffer {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VmaAllocation alloc = nullptr;
                VkBuffer resource = VK_NULL_HANDLE;

                struct Subresource {
                    bool typed = false;
                    VkBufferView view = VK_NULL_HANDLE;
                    VkDescriptorBufferInfo info = {};
                    int idx = -1;

                    constexpr bool IsValid() const { return idx >= 0; }
                };

                Subresource srv;
                Subresource uav;
                std::vector<Subresource> srvRes;
                std::vector<Subresource> uavRes;

                VkDeviceAddress ptr = 0;

                void DestroySubresources() {
                    size_t frames = manager->frames;
                    if (srv.IsValid()) {
                        if (srv.typed) {
                            manager->destroyer_bufferViews.emplace_back(srv.view, frames);
                            manager->destroyer_bindlessUniformTBuffers.emplace_back(srv.idx, frames);
                        } else {
                            manager->destroyer_bindlessStorageBuffers.emplace_back(srv.idx, frames);
                        }
                        srv = {};
                    }

                    if (uav.IsValid()) {
                        if (uav.typed) {
                            manager->destroyer_bufferViews.emplace_back(uav.view, frames);
                            manager->destroyer_bindlessStorageTBuffers.emplace_back(uav.idx, frames);
                        } else {
                            manager->destroyer_bindlessStorageBuffers.emplace_back(uav.idx, frames);
                        }
                        uav = {};
                    }

                    for (auto &x : srvRes) {
                        if (x.typed) {
                            manager->destroyer_bufferViews.emplace_back(x.view, frames);
                            manager->destroyer_bindlessUniformTBuffers.emplace_back(x.idx, frames);
                        } else {
                            manager->destroyer_bindlessStorageBuffers.emplace_back(x.idx, frames);
                        }
                    }
                    srvRes.clear();

                    for (auto &x : uavRes) {
                        if (x.typed) {
                            manager->destroyer_bufferViews.emplace_back(x.view, frames);
                            manager->destroyer_bindlessStorageTBuffers.emplace_back(x.idx, frames);
                        } else {
                            manager->destroyer_bindlessStorageBuffers.emplace_back(x.idx, frames);
                        }
                    }
                    uavRes.clear();
                }

                ~VulkanBuffer() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (resource)
                        manager->destroyer_buffers.emplace_back(std::make_pair(resource, alloc), frames);
                    else if (alloc)
                        manager->destroyer_allocations.emplace_back(alloc, frames);
                    DestroySubresources();
                }
            };

            // A Texture subtype that holds all the Vulkan-specific information for a normal Texture.
            struct VulkanTexture {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VmaAllocation alloc = nullptr;
                VkImage resource = VK_NULL_HANDLE;
                VkBuffer staging = VK_NULL_HANDLE;

                struct Subresource {
                    VkImageView view = VK_NULL_HANDLE;
                    int idx = -1;
                    size_t firstMip = 0;
                    size_t mips = 0;
                    size_t firstSlice = 0;
                    size_t slices = 0;

                    constexpr bool IsValid() const { return idx >= 0; }
                };

                Subresource srv, uav, rtv, dsv;
                size_t framebufferLayers = 0;
                std::vector<Subresource> srvRes, uavRes, rtvRes, dsvRes;

                std::vector<SubresourceMeta> mappedRes;
                SparseTextureMeta sparse;

                VkImageView videoView = VK_NULL_HANDLE;

                void DestroySubresources() {
                    size_t frames = manager->frames;
                    if (srv.IsValid()) {
                        manager->destroyer_imageViews.emplace_back(srv.view, frames);
                        manager->destroyer_bindlessImages.emplace_back(srv.idx, frames);
                        srv = {};
                    }

                    if (uav.IsValid()) {
                        manager->destroyer_imageViews.emplace_back(uav.view, frames);
                        manager->destroyer_bindlessStorageImages.emplace_back(uav.idx, frames);
                        uav = {};
                    }

                    if (rtv.IsValid()) {
                        manager->destroyer_imageViews.emplace_back(rtv.view, frames);
                        rtv = {};
                    }

                    if (dsv.IsValid()) {
                        manager->destroyer_imageViews.emplace_back(dsv.view, frames);
                        dsv = {};
                    }

                    for (auto x : srvRes) {
                        manager->destroyer_imageViews.emplace_back(x.view, frames);
                        manager->destroyer_bindlessImages.emplace_back(x.idx, frames);
                    }
                    srvRes.clear();

                    for (auto x : uavRes) {
                        manager->destroyer_imageViews.emplace_back(x.view, frames);
                        manager->destroyer_bindlessStorageImages.emplace_back(x.idx, frames);
                    }
                    uavRes.clear();

                    for (auto x : rtvRes) {
                        manager->destroyer_imageViews.emplace_back(x.view, frames);
                    }
                    rtvRes.clear();

                    for (auto x : dsvRes) {
                        manager->destroyer_imageViews.emplace_back(x.view, frames);
                    }
                    dsvRes.clear();
                }

                ~VulkanTexture() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (resource)
                        manager->destroyer_images.emplace_back(std::make_pair(resource, alloc), frames);
                    else if (staging)
                        manager->destroyer_buffers.emplace_back(std::make_pair(staging, alloc), frames);
                    else if (alloc)
                        manager->destroyer_allocations.emplace_back(alloc, frames);

                    if (videoView != VK_NULL_HANDLE)
                        manager->destroyer_imageViews.emplace_back(videoView, frames);
                    DestroySubresources();
                }
            };

            // A Sampler subtype that holds all the Vulkan-specific information for a normal Sampler.
            struct VulkanSampler {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkSampler resource = VK_NULL_HANDLE;
                int idx = -1;

                ~VulkanSampler() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (resource) manager->destroyer_samplers.emplace_back(resource, frames);
                    if (idx >= 0) manager->destroyer_bindlessSamplers.emplace_back(idx, frames);
                }
            };

            struct VulkanQueryHeap {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkQueryPool pool = VK_NULL_HANDLE;

                ~VulkanQueryHeap() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (pool) manager->destroyer_queryPools.emplace_back(pool, frames);
                }
            };

            struct VulkanShader {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkShaderModule shader = VK_NULL_HANDLE;
                VkPipeline computePipeline = VK_NULL_HANDLE;
                VkPipelineShaderStageCreateInfo stageCreate = {};
                VkPipelineLayout computeLayout = VK_NULL_HANDLE;
                VkDescriptorSetLayout layout = VK_NULL_HANDLE;
                std::vector<VkDescriptorSetLayoutBinding> bindings;
                std::vector<VkImageViewType> views;
                std::vector<TrackedBinding> bindless;
                std::vector<VkDescriptorSet> bindlessSets;
                size_t bindlessFirst = 0;

                VkPushConstantRange push = {};
                VkDeviceSize uniforms[BIND_CONSTANTS] = {};
                std::vector<size_t> uniformSlots;

                size_t bindHash = 0;

                ~VulkanShader() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (shader) manager->destroyer_shaders.emplace_back(shader, frames);
                    if (computePipeline) manager->destroyer_pipelines.emplace_back(computePipeline, frames);
                }
            };

            struct VulkanPipelineState {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkPipeline pipeline = VK_NULL_HANDLE;
                VkPipelineLayout layout = VK_NULL_HANDLE;
                VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
                std::vector<VkDescriptorSetLayoutBinding> bindings;
                std::vector<VkImageViewType> types;
                size_t hash = 0;

                std::vector<TrackedBinding> bindless;
                std::vector<VkDescriptorSet> bindlessSets;
                size_t bindlessFirst = 0;

                VkPushConstantRange push = {};

                VkDeviceSize uniforms[BIND_CONSTANTS] = {};
                std::vector<size_t> uniformSlots;

                VkGraphicsPipelineCreateInfo pipelineCreate = {};
                VkPipelineShaderStageCreateInfo stageCreates[static_cast<size_t>(ShaderStage::Size)] = {};
                VkPipelineInputAssemblyStateCreateInfo inputCreate = {};
                VkPipelineRasterizationStateCreateInfo rasterizerCreate = {};
                VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipCreate = {};
                VkPipelineViewportStateCreateInfo viewportCreate = {};
                VkPipelineDepthStencilStateCreateInfo depthStencilCreate = {};
                VkSampleMask sampleMask = {};
                VkPipelineTessellationStateCreateInfo tessellationCreate = {};

                ~VulkanPipelineState() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (pipeline) manager->destroyer_pipelines.emplace_back(pipeline, frames);
                }
            };

            struct VulkanRT {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VmaAllocation alloc = nullptr;
                VkBuffer buffer = VK_NULL_HANDLE;
                VkAccelerationStructureKHR res = VK_NULL_HANDLE;
                int idx = -1;

                VkAccelerationStructureBuildGeometryInfoKHR geometry = {};
                VkAccelerationStructureBuildSizesInfoKHR sizes = {};
                VkAccelerationStructureCreateInfoKHR create = {};
                std::vector<VkAccelerationStructureGeometryKHR> geos = {};
                std::vector<size_t> primitives;
                VkDeviceAddress scratch = 0;
                VkDeviceAddress addr = 0;

                ~VulkanRT() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (buffer) manager->destroyer_buffers.emplace_back(std::make_pair(buffer, alloc), frames);
                    if (res) manager->destroyer_rt.emplace_back(res, frames);
                    if (idx >= 0) manager->destroyer_bindlessRT.emplace_back(idx, frames);
                }
            };

            struct VulkanRTPipeline {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkPipeline pipeline;

                ~VulkanRTPipeline() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    if (pipeline) manager->destroyer_pipelines.emplace_back(pipeline, frames);
                }
            };

            struct VulkanSwapchain {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkSwapchainKHR swapchain = VK_NULL_HANDLE;
                VkFormat format;
                VkExtent2D extent;
                std::vector<VkImage> images;
                std::vector<VkImageView> views;

                Texture dummy;

                VkSurfaceKHR surface = VK_NULL_HANDLE;

                size_t imageIdx = 0;
                size_t acquireIdx = 0;
                std::vector<VkSemaphore> acquire;
                VkSemaphore release = VK_NULL_HANDLE;

                ColorSpace colorspace = ColorSpace::SRGB;
                SwapchainMeta meta;
                std::mutex lock;

                ~VulkanSwapchain() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;

                    for (size_t i = 0; i < images.size(); i++) {
                        manager->destroyer_imageViews.emplace_back(views[i], frames);
                        manager->destroyer_semaphores.emplace_back(acquire[i], frames);
                    }

                    if (SDL_WasInit(SDL_INIT_VIDEO)) {
                        manager->destroyer_swapChains.emplace_back(swapchain, frames);
                        manager->destroyer_surfaces.emplace_back(surface, frames);
                    }

                    manager->destroyer_semaphores.emplace_back(release, frames);
                }
            };

            struct VulkanDecoder {
                std::shared_ptr<VulkanInterface::MemoryManager> manager;
                VkVideoSessionKHR session = VK_NULL_HANDLE;
                VkVideoSessionParametersKHR parameters = VK_NULL_HANDLE;
                std::vector<VmaAllocation> allocs;

                ~VulkanDecoder() {
                    if (manager == nullptr) return;
                    std::scoped_lock lock(manager->destroyLock);

                    size_t frames = manager->frames;
                    manager->destroyer_videoSessions.emplace_back(session, frames);
                    manager->destroyer_videoSessionParameters.emplace_back(parameters, frames);
                    for (auto &x : allocs)
                        manager->destroyer_allocations.emplace_back(x, frames);
                }
            };

            VulkanBuffer *ToInternal(const GPUBuffer *buffer) {
                return static_cast<VulkanBuffer *>(buffer->internal.get());
            }

            VulkanTexture *ToInternal(const Texture *buffer) {
                return static_cast<VulkanTexture*>(buffer->internal.get());
            }

            VulkanSampler* ToInternal(const Sampler* buffer) {
                return static_cast<VulkanSampler*>(buffer->internal.get());
            }

            VulkanQueryHeap* ToInternal(const GPUQueryHeap* buffer) {
                return static_cast<VulkanQueryHeap*>(buffer->internal.get());
            }

            VulkanShader* ToInternal(const Shader* buffer) {
                return static_cast<VulkanShader*>(buffer->internal.get());
            }

            VulkanPipelineState* ToInternal(const PipelineState* buffer) {
                return static_cast<VulkanPipelineState*>(buffer->internal.get());
            }

            VulkanRT* ToInternal(const RaytracingAcceleration* buffer) {
                return static_cast<VulkanRT*>(buffer->internal.get());
            }

            VulkanRTPipeline* ToInternal(const RaytracingPipeline* buffer) {
                return static_cast<VulkanRTPipeline*>(buffer->internal.get());
            }

            VulkanSwapchain* ToInternal(const SwapChain* buffer) {
                return static_cast<VulkanSwapchain*>(buffer->internal.get());
            }

            VulkanDecoder* ToInternal(const VideoDecoder* buffer) {
                return static_cast<VulkanDecoder*>(buffer->internal.get());
            }
        }

        /**
         * @brief Methods to create the Vulkan specific specializations over abstract types.
         */
        namespace create {

            /**
             * @brief Handle the creation of a new, or an update of an existing, Swapchain.
             * @param internal the internal struct to create/update the contents of.
             * @param physical the physical device to be used for the swapchain
             * @param device the logical device to be used for the swapchain
             * @param manager the memory manager
             * @return whether the update or created completed successfully
             */
            bool Swapchain(structs::VulkanSwapchain* internal, VkPhysicalDevice physical, VkDevice device, std::shared_ptr<VulkanInterface::MemoryManager> manager) {
                // Lock everything, because we can never know what's being accessed while a recreate is called for.
                std::scoped_lock lock(internal->lock);

                VkResult result;

                // Fetch all existing data first
                VkSurfaceCapabilitiesKHR capabilities;
                result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, internal->surface, &capabilities);
                assert(result == VK_SUCCESS);

                uint32_t nFormats;
                result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical, internal->surface, &nFormats, nullptr);
                assert(result == VK_SUCCESS);
                std::vector<VkSurfaceFormatKHR> formats;
                result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical, internal->surface, &nFormats, formats.data());
                assert(result == VK_SUCCESS);

                uint32_t nModes;
                result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical, internal->surface, &nModes, nullptr);
                assert(result == VK_SUCCESS);
                std::vector<VkPresentModeKHR> modes;
                result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical, internal->surface, &nModes, modes.data());
                assert(result == VK_SUCCESS);

                // Figure out if we support the surface format and/or color space that the (re)create wants
                VkSurfaceFormatKHR format = {
                    .format = convert::Format(internal->meta.format),
                    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                };
                bool valid = false;

                for (const auto& f : formats) {
                    // Auto-reject HDR if we don't support it
                    if (!internal->meta.hdr && format.colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) continue;
                    // Accept first match
                    if (f.format == format.format) {
                        format = f; // Format will be the same, but .colorSpace might be different.
                        valid = true;
                        break;
                    }
                }

                // Fall back to u8 sRGB if we don't get what it wants
                if (!valid) {
                    internal->meta.format = ImageFormat::B8G8R8A8_UNORM;
                    format = {
                        .format = VK_FORMAT_B8G8R8A8_UNORM,
                        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                    };
                }

                // Make sure it didn't select a color space we're not compatible with, and save it back to the internal state.
                ColorSpace priorCS = internal->colorspace;
                switch (format.colorSpace) {
                    default: case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:    internal->colorspace = ColorSpace::SRGB;    break;
                    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:       internal->colorspace = ColorSpace::HDR16;   break;
                    case VK_COLOR_SPACE_HDR10_ST2084_EXT:               internal->colorspace = ColorSpace::HDR10;   break;
                }

                // Destroy the old swapchain if the color space has changed.
                if (priorCS != internal->colorspace) {
                    if (internal->swapchain != VK_NULL_HANDLE) {
                        // Wait for the swapchain to stop being used before we remove & recreate it.
                        result = vkDeviceWaitIdle(device);
                        assert(result == VK_SUCCESS);
                        vkDestroySwapchainKHR(device, internal->swapchain, nullptr);
                        internal->swapchain = nullptr;
                    }
                }

                // Clamp the size of the swapchain's extents
                if (capabilities.currentExtent.width != 0xFFFFFFFF && capabilities.currentExtent.height != 0xFFFFFFFF)
                    internal->extent = capabilities.currentExtent;
                else
                    internal->extent = {
                    std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, internal->meta.width)),
                    std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, internal->meta.height))
                };

                // Clamp the number of images in the swapchain
                uint32_t nImages = std::max(capabilities.minImageCount, std::min(capabilities.maxImageCount, internal->meta.buffers));

                // Prepare the creation info for the new swapchain
                VkSwapchainCreateInfoKHR create = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .surface = internal->surface,
                    .minImageCount = nImages,
                    .imageFormat = format.format,
                    .imageColorSpace = format.colorSpace,
                    .imageExtent = internal->extent,
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .preTransform = capabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = VK_PRESENT_MODE_FIFO_KHR,
                    .clipped = VK_TRUE,
                    .oldSwapchain = internal->swapchain
                  };

                // Mailbox and Immediate are only supported if vsync is not enabled.
                if (!internal->meta.vsync) {
                    for (auto& mode : modes) {
                        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                            create.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; break;
                        }

                        if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                            create.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; break;
                        }
                    }
                }

                // Make the new swapchain and update the old components
                result = vkCreateSwapchainKHR(device, &create, nullptr, &internal->swapchain);
                assert(result == VK_SUCCESS);

                if (create.oldSwapchain != VK_NULL_HANDLE) {
                    std::scoped_lock destroy(manager->destroyLock);
                    manager->destroyer_swapChains.emplace_back(create.oldSwapchain, manager->frames);
                }

                result = vkGetSwapchainImagesKHR(device, internal->swapchain, &nImages, nullptr);
                assert(result == VK_SUCCESS);
                result = vkGetSwapchainImagesKHR(device, internal->swapchain, &nImages, internal->images.data());
                assert(result == VK_SUCCESS);

                internal->format = format.format;
                internal->views.resize(internal->images.size());
                for (size_t i = 0; i < internal->images.size(); i++) {
                    VkImageViewCreateInfo viewCreate = {
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                        .image = internal->images[i],
                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                        .format = internal->format,
                        .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
                        .subresourceRange = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        }
                    };

                    if (internal->images[i] != VK_NULL_HANDLE) {
                        std::scoped_lock destroy(manager->destroyLock);
                        manager->destroyer_imageViews.emplace_back(internal->views[i], manager->frames);
                    }

                    result = vkCreateImageView(device, &viewCreate, nullptr, &internal->views[i]);
                    assert(result == VK_SUCCESS);
                }

                VkSemaphoreCreateInfo semaphoreCreate = {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
                };

                if (internal->acquire.empty())
                    for (size_t i = 0; i < internal->images.size(); i++) {
                        result = vkCreateSemaphore(device, &semaphoreCreate, nullptr, &internal->acquire.emplace_back());
                        assert(result == VK_SUCCESS);
                    }

                if (internal->release == VK_NULL_HANDLE) {
                    result = vkCreateSemaphore(device, &semaphoreCreate, nullptr, &internal->release);
                    assert(result == VK_SUCCESS);
                }

                return true;
            }
        }
    }

    static inline size_t NextPowerOfTwo(size_t v) { v--; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; v++; return v; }

    void VulkanInterface::CommandQueue::Submit(rx::VulkanInterface *iface, VkFence fence) {
        if (queue == VK_NULL_HANDLE) return;
        std::scoped_lock lock(*locker);

        VkSubmitInfo2 submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphoreSubmit.size()),
            .pWaitSemaphoreInfos = waitSemaphoreSubmit.data(),
            .commandBufferInfoCount = static_cast<uint32_t>(commandSubmit.size()),
            .pCommandBufferInfos = commandSubmit.data(),
            .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphoreSubmit.size()),
            .pSignalSemaphoreInfos = signalSemaphoreSubmit.data()
        };

        VkResult result = vkQueueSubmit2(queue, 1, &submit, fence);
        assert(result == VK_SUCCESS);

        // Submit the swapchain AFTER the queue, if any are bound.
        if (!swapchainSubmit.empty()) {
            VkPresentInfoKHR present = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size()),
                .pWaitSemaphores = signalSemaphores.data(),
                .swapchainCount = static_cast<uint32_t>(swapchainSubmit.size()),
                .pSwapchains = swapchainSubmit.data(),
                .pImageIndices = swapchainSubmitImages.data()
            };

            // If the window has been resized, or moved to a monitor with a different color space, this is where we'll be told about that.
            // We recreate the swapchain for suboptimal or out of date return values.
            // Other values are errors with firmware or hardware, so they are not recoverable.
            result = vkQueuePresentKHR(queue, &present);

            switch(result) {
                case VK_SUCCESS: break;
                case VK_SUBOPTIMAL_KHR:
                case VK_ERROR_OUT_OF_DATE_KHR: {
                    for (auto& sc : swapchainUpdates) {
                        auto internal = vulkan::structs::ToInternal(&sc);
                        bool success = vulkan::create::Swapchain(internal, iface->physicalDevice, iface->device, iface->memoryManager);
                        assert(success);
                    }
                }
                default:
                    assert(0);
            }
        }

        swapchainUpdates.clear();
        swapchainSubmit.clear();
        swapchainSubmitImages.clear();
        waitSemaphoreSubmit.clear();
        signalSemaphores.clear();
        signalSemaphoreSubmit.clear();
        commandSubmit.clear();
    }

    void VulkanInterface::CommandQueue::Signal(VkSemaphore sem) {
        if (queue == VK_NULL_HANDLE) return;
        signalSemaphoreSubmit.emplace_back(VkSemaphoreSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = sem,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
        });
    }

    void VulkanInterface::CommandQueue::Wait(VkSemaphore sem) {
        if (queue == VK_NULL_HANDLE) return;
        waitSemaphoreSubmit.emplace_back(VkSemaphoreSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = sem,
            .value = 0,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
        });
    }

    void VulkanInterface::Uploader::Init(rx::VulkanInterface *iface) {
        this->iface = iface;
    }

    void VulkanInterface::Uploader::Destroy() {
        // Wait for all queued uploads to finish first
        vkQueueWaitIdle(iface->queues[QueueType::COPY].queue);

        for (auto& x : freeList) {
            vkDestroyCommandPool(iface->device, x.transferPool, nullptr);
            vkDestroyCommandPool(iface->device, x.transitionPool, nullptr);
            for (auto& sem : x.semaphores)
                vkDestroySemaphore(iface->device, sem, nullptr);
            vkDestroyFence(iface->device, x.fence, nullptr);
        }
    }

    VulkanInterface::Uploader::Copy VulkanInterface::Uploader::Allocate(size_t staging) {
        Copy cmd;

        // Search for an existing buffer big enough
        {
            std::scoped_lock lock(locker);

            for (auto &buf : freeList) {
                if (buf.uploadBuffer.meta.size >= staging) {
                    if (vkGetFenceStatus(iface->device, buf.fence) == VK_SUCCESS) {
                        cmd = std::move(buf);
                        std::swap(buf, freeList.back());
                        freeList.pop_back();
                        break; // Won't error if we stop after removing
                    }
                }
            }
        }

        // No buffer big enough - create one
        if (!cmd.IsValid()) {
            VkCommandPoolCreateInfo create = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = static_cast<uint32_t>(iface->copyFamily)
            };

            VkResult result = vkCreateCommandPool(iface->device, &create, nullptr, &cmd.transferPool);
            assert(result == VK_SUCCESS);

            create.queueFamilyIndex = static_cast<uint32_t>(iface->graphicsFamily);

            result = vkCreateCommandPool(iface->device, &create, nullptr, &cmd.transitionPool);
            assert(result == VK_SUCCESS);

            VkCommandBufferAllocateInfo allocate = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = cmd.transferPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            result = vkAllocateCommandBuffers(iface->device, &allocate, &cmd.transferBuffer);
            assert(result == VK_SUCCESS);

            allocate.commandPool = cmd.transitionPool;
            result = vkAllocateCommandBuffers(iface->device, &allocate, &cmd.transitionBuffer);
            assert(result == VK_SUCCESS);

            VkFenceCreateInfo fence = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            result = vkCreateFence(iface->device, &fence, nullptr, &cmd.fence);
            assert(result == VK_SUCCESS);

            VkSemaphoreCreateInfo sem = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            result = vkCreateSemaphore(iface->device, &sem, nullptr, &cmd.semaphores[0]);
            assert(result == VK_SUCCESS);
            result = vkCreateSemaphore(iface->device, &sem, nullptr, &cmd.semaphores[1]);
            assert(result == VK_SUCCESS);
            result = vkCreateSemaphore(iface->device, &sem, nullptr, &cmd.semaphores[2]);
            assert(result == VK_SUCCESS);

            GPUBufferMeta meta = {
                .size = std::max(NextPowerOfTwo(staging), static_cast<size_t>(65536)),
                .usage = BufferUsage::STAGING
            };

            bool success = iface->CreateBuffer(&meta, nullptr, &cmd.uploadBuffer);
            assert(success);
            iface->SetName(&cmd.uploadBuffer, "Uploader staging buffer");
        }

        // Ensure all states are valid before passing them on to be used
        VkResult result = vkResetCommandPool(iface->device, cmd.transferPool, 0);
        assert(result == VK_SUCCESS);
        result = vkResetCommandPool(iface->device, cmd.transitionPool, 0);
        assert(result == VK_SUCCESS);

        VkCommandBufferBeginInfo begin = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        result = vkBeginCommandBuffer(cmd.transferBuffer, &begin);
        assert(result == VK_SUCCESS);
        result = vkBeginCommandBuffer(cmd.transitionBuffer, &begin);
        assert(result == VK_SUCCESS);

        result = vkResetFences(iface->device, 1, &cmd.fence);
        assert(result == VK_SUCCESS);

        return cmd;
    }

    void VulkanInterface::Uploader::Submit(rx::VulkanInterface::Uploader::Copy cmd) {
        VkResult result = vkEndCommandBuffer(cmd.transferBuffer);
        assert(result == VK_SUCCESS);
        result = vkEndCommandBuffer(cmd.transitionBuffer);
        assert(result == VK_SUCCESS);

        VkSubmitInfo2 submit = { VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
        VkCommandBufferSubmitInfo commandSubmit = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
        VkSemaphoreSubmitInfo signalSemaphoreSubmit[2] = { {VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO }, { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO } };
        VkSemaphoreSubmitInfo waitSemaphoreSubmit = { VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };

        // Submit transfer buffer
        {
            commandSubmit.commandBuffer = cmd.transferBuffer;
            signalSemaphoreSubmit[0].semaphore = cmd.semaphores[0];                                       // Signal the graphics queue when the copy queue is finished
            signalSemaphoreSubmit[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            submit.commandBufferInfoCount = 1;
            submit.pCommandBufferInfos = &commandSubmit;
            submit.signalSemaphoreInfoCount = 1;
            submit.pSignalSemaphoreInfos = signalSemaphoreSubmit;

            std::scoped_lock lock(*iface->queues[QueueType::COPY].locker);
            result = vkQueueSubmit2(iface->queues[QueueType::COPY].queue, 1, &submit, VK_NULL_HANDLE);
            assert(result == VK_SUCCESS);
        }

        // Submit graphics queue to transition the new buffer to a form the GPU can read
        {
            commandSubmit.commandBuffer = cmd.transitionBuffer;
            waitSemaphoreSubmit.semaphore = cmd.semaphores[0];                                            // Wait for the copy queue to signal it's finished
            waitSemaphoreSubmit.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            signalSemaphoreSubmit[0].semaphore = cmd.semaphores[1];                                       // Signal the compute queue when the graphics queue is finished
            signalSemaphoreSubmit[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            submit.waitSemaphoreInfoCount = 1;
            submit.pWaitSemaphoreInfos = &waitSemaphoreSubmit;
            submit.commandBufferInfoCount = 1;
            submit.pCommandBufferInfos = &commandSubmit;

            if (iface->queues[QueueType::VIDEO_DECODE].queue != VK_NULL_HANDLE) {
                signalSemaphoreSubmit[1].semaphore = cmd.semaphores[2];                                   // Signal the video decode queue when the graphics queue is finished
                signalSemaphoreSubmit[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
                submit.signalSemaphoreInfoCount = 2;
            } else {
                submit.signalSemaphoreInfoCount = 1;
            }
            submit.pSignalSemaphoreInfos = signalSemaphoreSubmit;

            std::scoped_lock lock(*iface->queues[QueueType::GRAPHICS].locker);
            result = vkQueueSubmit2(iface->queues[QueueType::GRAPHICS].queue, 1, &submit, VK_NULL_HANDLE);
            assert(result == VK_SUCCESS);
        }

        // Process the video decode buffer immediately, in case we just uploaded more video data.
        if (iface->queues[QueueType::VIDEO_DECODE].queue != VK_NULL_HANDLE) {
            waitSemaphoreSubmit.semaphore = cmd.semaphores[2];                                            // Wait for the graphics queue to signal it's finished
            waitSemaphoreSubmit.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            submit.waitSemaphoreInfoCount = 1;
            submit.pWaitSemaphoreInfos = &waitSemaphoreSubmit;
            submit.commandBufferInfoCount = 0;
            submit.pCommandBufferInfos = nullptr;
            submit.signalSemaphoreInfoCount = 0;
            submit.pSignalSemaphoreInfos = nullptr;

            std::scoped_lock lock(*iface->queues[QueueType::VIDEO_DECODE].locker);
            result = vkQueueSubmit2(iface->queues[QueueType::VIDEO_DECODE].queue, 1, &submit, VK_NULL_HANDLE);
            assert(result == VK_SUCCESS);
        }

        // Process the compute queue last so we can signal the CPU that the buffer is completely finished
        {
            waitSemaphoreSubmit.semaphore = cmd.semaphores[1];                                            // Wait for the graphics queue to signal it's finished
            waitSemaphoreSubmit.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

            submit.waitSemaphoreInfoCount = 1;
            submit.pWaitSemaphoreInfos = &waitSemaphoreSubmit;
            submit.commandBufferInfoCount = 0;
            submit.pCommandBufferInfos = nullptr;
            submit.signalSemaphoreInfoCount = 0;
            submit.pSignalSemaphoreInfos = nullptr;

            std::scoped_lock lock(*iface->queues[QueueType::COMPUTE].locker);
            result = vkQueueSubmit2(iface->queues[QueueType::COMPUTE].queue, 1, &submit, cmd.fence);      // Raise the fence to signal the completion of the buffer upload
            assert(result == VK_SUCCESS);
        }

        std::scoped_lock lock(locker);
        freeList.push_back(cmd);
    }

    void VulkanInterface::DescriptorPool::Init(rx::VulkanInterface *iface) {
        this->iface = iface;
        VkResult result;

        VkDescriptorPoolSize sizes[10] = {};
        size_t cnt = 0;

        sizes[0] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, BIND_CONSTANTS * poolSize };
        sizes[1] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, BIND_CONSTANTS * poolSize };
        sizes[2] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, BIND_SHADERS * poolSize };
        sizes[3] = { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, BIND_SHADERS * poolSize };
        sizes[4] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, BIND_SHADERS * poolSize };
        sizes[5] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, BIND_UNIFORMS * poolSize };
        sizes[6] = { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, BIND_UNIFORMS * poolSize };
        sizes[7] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, BIND_UNIFORMS * poolSize };
        sizes[8] = { VK_DESCRIPTOR_TYPE_SAMPLER, BIND_SAMPLERS * poolSize };

        if (iface->CheckCapability(GraphicsDeviceCapability::RAY_TRACING)) {
            sizes[9] = { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, BIND_SHADERS * poolSize };
            cnt = 10;
        } else {
            cnt = 9;
        }

        VkDescriptorPoolCreateInfo create = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = poolSize,
            .poolSizeCount = static_cast<uint32_t>(cnt),
            .pPoolSizes = sizes,
        };

        result = vkCreateDescriptorPool(iface->device, &create, nullptr, &pool);
        assert(result == VK_SUCCESS);
    }

    void VulkanInterface::DescriptorPool::Destroy() {
        if (pool != VK_NULL_HANDLE) {
            std::scoped_lock lock(iface->memoryManager->destroyLock);
            iface->memoryManager->destroyer_descriptorPools.emplace_back(pool, iface->frameCount);
            pool = VK_NULL_HANDLE;
        }
    }

    void VulkanInterface::DescriptorPool::Reset() {
        if (pool != VK_NULL_HANDLE)
            assert(vkResetDescriptorPool(iface->device, pool, 0) == VK_SUCCESS);
    }

    void VulkanInterface::DescriptorBind::Init(rx::VulkanInterface *iface) {
        this->iface = iface;

        descriptorWrite.reserve(128);
        buffers.reserve(128);
        images.reserve(128);
        views.reserve(128);
        accelerationStructures.reserve(128);
    }

    void VulkanInterface::DescriptorBind::Reset() {
        table = {};
        dirty = DirtyFlags::ALL;
    }

    void VulkanInterface::DescriptorBind::Flush(bool graphics, rx::ThreadCommands cmd) {
        if (dirty == DirtyFlags::NONE) return;

        // Set some useful state to be used later
        VulkanThreadCommands const& commands = iface->GetThreadCommands(cmd);
        auto pso = graphics ? vulkan::structs::ToInternal(commands.activePSO) : nullptr;
        auto compute = graphics ? nullptr : vulkan::structs::ToInternal(commands.activeShader);
        const auto& bindings = graphics ? pso->bindings : compute->bindings;
        const auto& viewTypes = graphics ? pso->types : compute->views;
        VkCommandBuffer buffer = commands.GetCommandBuffer();

        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
        VkDescriptorSet set = VK_NULL_HANDLE;
        size_t uniformDynamics = 0;

        if (graphics) {
            layout = pso->layout;
            setLayout = pso->setLayout;
            set = this->graphicsSet;
            uniformDynamics = static_cast<uint32_t>(pso->uniformSlots.size());
            for (size_t i = 0; i < pso->uniformSlots.size(); i++)
                uniformOffsets[i] = static_cast<uint32_t>(table.constantOffsets[pso->uniformSlots[i]]);
        } else {
            layout = compute->computeLayout;
            setLayout = compute->layout;
            set = this->computeSet;
            uniformDynamics = static_cast<uint32_t>(compute->uniformSlots.size());
            for (size_t i = 0; i < compute->uniformSlots.size(); i++)
                uniformOffsets[i] = static_cast<uint32_t>(table.constantOffsets[compute->uniformSlots[i]]);
        }

        // Update descriptors if necessary
        if (static_cast<uint32_t>(dirty & DirtyFlags::DESCRIPTOR)) {
            auto& pool = (DescriptorPool&) (commands.bindPools[iface->GetBufferIndex()]);

            VkDescriptorSetAllocateInfo allocate = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = pool.pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &setLayout
            };

            VkResult result = vkAllocateDescriptorSets(iface->device, &allocate, &set);
            // Try bigger pool sizes until it works.
            // TODO: This is an endless loop, try graceful handling of out of memory at the largest size? Should return OUT_OF_DEVICE_MEMORY and hit the assert, but still..
            while (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
                pool.Destroy();
                pool.poolSize *= 2;
                pool.Init(iface);
                allocate.descriptorPool = pool.pool;
                result = vkAllocateDescriptorSets(iface->device, &allocate, &set);
            }
            assert(result == VK_SUCCESS);

            descriptorWrite.clear();
            buffers.clear();
            images.clear();
            views.clear();
            accelerationStructures.clear();

            int idx = 0;
            for (auto& bind : bindings) {
                if (bind.pImmutableSamplers != nullptr) {
                    idx++; continue;
                }

                VkImageViewType viewtype = viewTypes[idx++];

                for (uint32_t descriptorIdx = 0; descriptorIdx < bind.descriptorCount; descriptorIdx++) {
                    uint32_t unrolled = bind.binding + descriptorIdx;

                    VkWriteDescriptorSet write = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = set,
                        .dstArrayElement = descriptorIdx,
                        .descriptorType = bind.descriptorType,
                        .dstBinding = bind.binding,
                        .descriptorCount = 1
                      };

                    descriptorWrite.emplace_back(write);

                    switch (bind.descriptorType) {
                        case VK_DESCRIPTOR_TYPE_SAMPLER: {
                            const Sampler& sampler = table.samplers[unrolled - vulkan::BindShift::SAMPLER];
                            images.emplace_back(VkDescriptorImageInfo { .sampler = sampler.IsValid() ? vulkan::structs::ToInternal(&sampler)->resource : iface->nullSampler });
                            write.pImageInfo = &images.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
                            const GPUResource& res = table.shaders[unrolled - vulkan::BindShift::TEXTURE];
                            images.emplace_back(VkDescriptorImageInfo{});

                            if (!res.IsValid() || !res.isTexture()) {
                                images.back().imageView =
                                    viewtype == VK_IMAGE_VIEW_TYPE_1D         ? iface->nullImageView1     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_2D         ? iface->nullImageView2     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_3D         ? iface->nullImageView3     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_CUBE       ? iface->nullImageViewC     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_1D_ARRAY   ? iface->nullImageView1A    :
                                    viewtype == VK_IMAGE_VIEW_TYPE_2D_ARRAY   ? iface->nullImageView2A    :
                                    viewtype == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY ? iface->nullImageViewCA    :
                                                                                iface->nullImageView1;
                                images.back().imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            } else {
                                int sub = table.shaderIndex[unrolled - vulkan::BindShift::TEXTURE];
                                auto tex = vulkan::structs::ToInternal(static_cast<const Texture*>(&res));
                                images.back().imageView = sub >= 0 ? tex->srvRes[sub] : tex->srv.view;
                                images.back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            }
                            write.pImageInfo = &images.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
                            const GPUResource& res = table.uniforms[unrolled - vulkan::BindShift::UNIFORM];
                            images.emplace_back(VkDescriptorImageInfo{ .imageLayout = VK_IMAGE_LAYOUT_GENERAL });

                            if (!res.IsValid() || !res.isTexture()) {
                                images.back().imageView =
                                    viewtype == VK_IMAGE_VIEW_TYPE_1D         ? iface->nullImageView1     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_2D         ? iface->nullImageView2     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_3D         ? iface->nullImageView3     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_CUBE       ? iface->nullImageViewC     :
                                    viewtype == VK_IMAGE_VIEW_TYPE_1D_ARRAY   ? iface->nullImageView1A    :
                                    viewtype == VK_IMAGE_VIEW_TYPE_2D_ARRAY   ? iface->nullImageView2A    :
                                    viewtype == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY ? iface->nullImageViewCA    :
                                                                                iface->nullImageView1;
                            } else {
                                int sub = table.uniformIndex[unrolled - vulkan::BindShift::UNIFORM];
                                auto tex = vulkan::structs::ToInternal(static_cast<const Texture*>(&res));
                                images.back().imageView = sub >= 0 ? tex->uavRes[sub] : tex->uav.view;
                            }
                            write.pImageInfo = &images.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                            const GPUBuffer& buf = table.constants[unrolled - vulkan::BindShift::BUFFER];

                            buffers.emplace_back( VkDescriptorBufferInfo {
                              .buffer = buf.IsValid() ? vulkan::structs::ToInternal(&buf)->resource : iface->nullBuffer,
                              .offset = buf.IsValid() ? table.constantOffsets[unrolled - vulkan::BindShift::BUFFER] : 0,
                              .range = !buf.IsValid() ? VK_WHOLE_SIZE :
                                  graphics ? pso->uniforms[unrolled - vulkan::BindShift::BUFFER] : compute->uniforms[unrolled - vulkan::BindShift::BUFFER]
                            });
                            // If the uniform size in either PSO or Compute Shader are zero, we manually set it to the whole range.
                            buffers.back().range = buffers.back().range == 0 ? VK_WHOLE_SIZE : buffers.back().range;
                            write.pBufferInfo = &buffers.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
                            const GPUResource& res = table.shaders[unrolled - vulkan::BindShift::TEXTURE];

                            views.emplace_back(
                                (res.IsValid() && res.isBuffer()) ?
                                  table.shaderIndex[unrolled - vulkan::BindShift::TEXTURE] >= 0 ?
                                      vulkan::structs::ToInternal((const GPUBuffer*)(&res))->srvRes[table.shaderIndex[unrolled - vulkan::BindShift::TEXTURE]].view :
                                      vulkan::structs::ToInternal((const GPUBuffer*)(&res))->srv.view
                                  : iface->nullBufferView
                            );
                            write.pTexelBufferView = &views.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
                            const GPUResource& res = table.uniforms[unrolled - vulkan::BindShift::UNIFORM];

                            views.emplace_back(
                                (res.IsValid() && res.isBuffer()) ?
                                  table.uniformIndex[unrolled - vulkan::BindShift::UNIFORM] >= 0 ?
                                      vulkan::structs::ToInternal((const GPUBuffer*)&res)->uavRes[table.uniformIndex[unrolled - vulkan::BindShift::UNIFORM]].view :
                                      vulkan::structs::ToInternal((const GPUBuffer*)&res)->uav.view
                                  : iface->nullBufferView
                            );
                            write.pTexelBufferView = &views.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
                            if (bind.binding < vulkan::BindShift::UNIFORM) {
                                const GPUBuffer& buf = table.constants[unrolled - vulkan::BindShift::BUFFER];
                                buffers.emplace_back( (buf.IsValid() && buf.isBuffer()) ? VkDescriptorBufferInfo { .buffer = iface->nullBuffer, .range = VK_WHOLE_SIZE} :
                                  table.shaderIndex[unrolled - vulkan::BindShift::BUFFER] >= 0 ?
                                      vulkan::structs::ToInternal(&buf)->srvRes[table.shaderIndex[unrolled - vulkan::BindShift::BUFFER]].info :
                                      vulkan::structs::ToInternal(&buf)->srv.info
                                );
                            } else {
                                const GPUResource& res = table.uniforms[unrolled - vulkan::BindShift::UNIFORM];
                                buffers.emplace_back( (res.IsValid() && res.isBuffer()) ? VkDescriptorBufferInfo { .buffer = iface->nullBuffer, .range = VK_WHOLE_SIZE} :
                                  table.uniformIndex[unrolled - vulkan::BindShift::UNIFORM] >= 0 ?
                                      vulkan::structs::ToInternal((const GPUBuffer*)&res)->uavRes[table.uniformIndex[unrolled - vulkan::BindShift::UNIFORM]].info :
                                      vulkan::structs::ToInternal((const GPUBuffer*)&res)->uav.info
                                );
                            }
                            write.pBufferInfo = &buffers.back();
                            break;
                        }

                        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
                            accelerationStructures.emplace_back(VkWriteDescriptorSetAccelerationStructureKHR {
                                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                                .accelerationStructureCount = 1,
                                .pAccelerationStructures = &vulkan::structs::ToInternal((const RaytracingAcceleration*)&table.shaders[unrolled - vulkan::BindShift::TEXTURE])->res
                            });
                            write.pNext = &accelerationStructures.back();
                        }

                        default: break;
                    }
                }
            }

            vkUpdateDescriptorSets(iface->device, (uint32_t) descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
        }

        vkCmdBindDescriptorSets(buffer,
            graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS :
              commands.activeShader->stage == ShaderStage::LIB ?
                  VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR :
                  VK_PIPELINE_BIND_POINT_COMPUTE,
            layout, 0, 1, &set, uniformDynamics, uniformOffsets);

        if (graphics) graphicsSet = set;
        else computeSet = set;

        dirty = DirtyFlags::NONE;
    }

    void VulkanInterface::ValidatePSOs(ThreadCommands cmd) {
        VulkanThreadCommands& command = GetThreadCommands(cmd);
        if (!command.PSODirty) return;

        const auto* pso = command.activePSO;
        size_t hash = command.prevPipelineHash;
        auto internal = vulkan::structs::ToInternal(pso);

        VkPipeline pipeline = VK_NULL_HANDLE;
        // Find the pipeline in the hash if possible
        auto iter = pipelines.find(hash);
        // If not in the hash, it's either just been created, or is stale.
        if (iter == pipelines.end()) {
            // Look for the pipeline in the workers
            for (auto& p : command.pipelines) {
                if (hash == p.first) {
                    pipeline = p.second; break;
                }
            }

            // If we didn't just find it, then create the pipeline using the stored info
            if (pipeline == VK_NULL_HANDLE) {
                auto create = internal->pipelineCreate;

                // Setup multisampling
                VkPipelineMultisampleStateCreateInfo msCreate = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .sampleShadingEnable = VK_FALSE,
                    .rasterizationSamples = (pso->meta.rasterizer != nullptr && pso->meta.rasterizer->forcedSampleCount > 1) ?
                        (VkSampleCountFlagBits) pso->meta.rasterizer->forcedSampleCount :
                        (VkSampleCountFlagBits) command.passMeta.sampleCount,
                    .minSampleShading = 1.f,
                    .pSampleMask = &pso->meta.sampleMask,
                    .alphaToCoverageEnable = pso->meta.blend != nullptr ?
                        pso->meta.blend->alpha ? VK_TRUE : VK_FALSE :
                        VK_FALSE,
                    .alphaToOneEnable = VK_FALSE
                };

                create.pMultisampleState = &msCreate;

                // Setup blending
                uint32_t blendedAttachments = 0;
                VkPipelineColorBlendAttachmentState blendAttach[8] = {};
                for (size_t i = 0; i < command.passMeta.targetCount; i++) {
                    size_t idx = 0;
                    if (pso->meta.blend->independent) idx = i;

                    const auto& target = pso->meta.blend->targets[idx];
                    auto& attach = blendAttach[blendedAttachments++];
                    attach.blendEnable = target.enable ? VK_TRUE : VK_FALSE;
                    attach.colorWriteMask = 0;
                    if (has_flag(target.writeMask, ColorWrite::ENABLE_RED)) attach.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
                    if (has_flag(target.writeMask, ColorWrite::ENABLE_GREEN)) attach.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
                    if (has_flag(target.writeMask, ColorWrite::ENABLE_BLUE)) attach.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
                    if (has_flag(target.writeMask, ColorWrite::ENABLE_ALPHA)) attach.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
                    attach.srcColorBlendFactor = vulkan::convert::BlendSource(target.source);
                    attach.dstColorBlendFactor = vulkan::convert::BlendSource(target.dest);
                    attach.colorBlendOp = vulkan::convert::BlendOperation(target.op);
                    attach.srcAlphaBlendFactor = vulkan::convert::BlendSource(target.sourceAlpha);
                    attach.dstAlphaBlendFactor = vulkan::convert::BlendSource(target.destAlpha);
                    attach.alphaBlendOp = vulkan::convert::BlendOperation(target.opAlpha);
                }

                VkPipelineColorBlendStateCreateInfo blend = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    .logicOpEnable = VK_FALSE,
                    .logicOp = VK_LOGIC_OP_COPY,
                    .attachmentCount = blendedAttachments,
                    .pAttachments = blendAttach,
                    .blendConstants = { 1, 1, 1, 1 }
                };

                create.pColorBlendState = &blend;

                // Setup Vertex input
                VkPipelineVertexInputStateCreateInfo vertexCreate = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
                };
                std::vector<VkVertexInputBindingDescription> bind;
                std::vector<VkVertexInputAttributeDescription> attr;

                if (pso->meta.layout != nullptr) {
                    uint32_t lastBind = ~0ull, lastAttr = ~0ull, idx = 0, off = 0;
                    for (auto& element : pso->meta.layout->elements) {
                        if (element.slot == lastBind) continue; // Skip duplicate bindings
                        if (element.slot != lastAttr) {         // Process duplicate attributes
                            lastAttr = element.slot;
                            off = 0;
                        }
                        lastBind = element.slot;
                        bind.emplace_back(VkVertexInputBindingDescription { element.slot, GetFormatStride(element.format), element.slotClass == InputClassification::VERTEX_DATA ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE});
                        attr.emplace_back(VkVertexInputAttributeDescription { idx, element.slot, vulkan::convert::Format(element.format), element.alignedOffset });
                        // Process the aligned element
                        if (attr.back().offset == InputLayout::ALIGNED_ELEMENT) {
                            attr.back().offset = off;
                            off += GetFormatStride(element.format);
                        }
                        idx++;
                    }

                    vertexCreate.vertexBindingDescriptionCount = static_cast<uint32_t>(bind.size());
                    vertexCreate.pVertexBindingDescriptions = bind.data();
                    vertexCreate.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
                    vertexCreate.pVertexAttributeDescriptions = attr.data();
                }
                create.pVertexInputState = &vertexCreate;

                // Setup Rendering
                create.renderPass = VK_NULL_HANDLE;
                VkFormat formats[8] = {};
                for (uint32_t i = 0; i < command.passMeta.targetCount; i++)
                    formats[i] = vulkan::convert::Format(command.passMeta.targetFormats[i]);
                VkPipelineRenderingCreateInfo rendering = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .viewMask = 0,
                    .colorAttachmentCount = command.passMeta.targetCount,
                    .pColorAttachmentFormats = formats,
                    .depthAttachmentFormat = vulkan::convert::Format(command.passMeta.depthStencilFormat),
                    .stencilAttachmentFormat = IsFormatStencil(command.passMeta.depthStencilFormat) ? vulkan::convert::Format(command.passMeta.depthStencilFormat) : VK_FORMAT_UNDEFINED
                };
                create.pNext = &rendering;

                VkResult res = vkCreateGraphicsPipelines(device, pipelineCache, 1, &create, nullptr, &pipeline);
                assert(res == VK_SUCCESS);
                command.pipelines.emplace_back(hash, pipeline);
            }
        // No pipeline in cache
        } else {
            pipeline = iter->second;
        }

        assert(pipeline != VK_NULL_HANDLE);

        vkCmdBindPipeline(command.GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        command.PSODirty = false;
    }

    void VulkanInterface::Predraw(ThreadCommands cmd) {
        ValidatePSOs(cmd);
        GetThreadCommands(cmd).binds.Flush(true, cmd);
    }

    void VulkanInterface::Predispatch(ThreadCommands cmd) {
        GetThreadCommands(cmd).binds.Flush(false, cmd);
    }

    std::vector<const char*> VulkanInterface::ProcessPhysicalDevice(VkPhysicalDevice dev) {
        const std::vector<const char*> requiredExts = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        std::vector<const char*> enabledExts;

        uint32_t nExts;
        VkResult res = vkEnumerateDeviceExtensionProperties(dev, nullptr, &nExts, nullptr);
        assert(res == VK_SUCCESS);
        std::vector<VkExtensionProperties> exts(nExts);
        res = vkEnumerateDeviceExtensionProperties(dev, nullptr, &nExts, exts.data());
        assert(res == VK_SUCCESS);

        for (auto& ex : requiredExts)
            if (!vulkan::CheckExtension(ex, exts))
                return enabledExts;

        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceFeatures11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        deviceProps11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        deviceFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        deviceProps12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        deviceFeatures13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        deviceProps13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;

        deviceFeatures2.pNext = &deviceFeatures11;
        deviceFeatures11.pNext = &deviceFeatures12;
        deviceFeatures12.pNext = &deviceFeatures13;
        void** features = &deviceFeatures13.pNext;
        deviceProps2.pNext = &deviceProps11;
        deviceProps11.pNext = &deviceProps12;
        deviceProps12.pNext = &deviceProps13;
        void** props = &deviceProps13.pNext;

        rayTracingFeatures = {};
        rayTracingProps = {};
        accelerationStructureFeatures = {};
        accelerationStructureProps = {};
        rayTracingQueryFeatures = {};
        fragmentShadingRateFeatures = {};
        fragmentShadingRateProps = {};
        meshShaderFeatures = {};
        meshShaderProps = {};
        conditionalRenderingFeatures = {};
        depthClipEnableFeatures = {};
        samplerMinmaxProps = {};

        #define APPEND_PROPERTIES_CHAIN(x, y)   \
        x##.sType = y;                          \
        *props = &x;                            \
        props = &x##.pNext


        #define APPEND_FEATURES_CHAIN(x, y)     \
        x##.sType = y;                          \
        *features = &x;                         \
        features = &x##.pNext


        APPEND_PROPERTIES_CHAIN(samplerMinmaxProps, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES);
        APPEND_PROPERTIES_CHAIN(depthStencilResolveProps, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES);

        enabledExts = requiredExts;

        #define ENABLE_IF_AVAILABLE(ext, code)      \
        if (vulkan::CheckExtension(ext, exts)) {    \
            enabledExts.push_back(ext);             \
            code                                    \
        }

        ENABLE_IF_AVAILABLE(VK_EXT_IMAGE_VIEW_MIN_LOD_EXTENSION_NAME,
            APPEND_FEATURES_CHAIN(minLodFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_VIEW_MIN_LOD_FEATURES_EXT);
        )

        ENABLE_IF_AVAILABLE(VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME,
            APPEND_FEATURES_CHAIN(depthClipEnableFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT);
        )

        ENABLE_IF_AVAILABLE(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            // We use deferred-host operations to implement bounding volume hierarchies, so that's a hard requirement
            assert(vulkan::CheckExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, exts));
            enabledExts.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            APPEND_FEATURES_CHAIN(accelerationStructureFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR);
            APPEND_PROPERTIES_CHAIN(accelerationStructureProps, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR);

            // BVH support implies two optional extensions, enable them if available.
            ENABLE_IF_AVAILABLE(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                // If the pipeline is support, the pipeline library is by default.
                enabledExts.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
                APPEND_FEATURES_CHAIN(rayTracingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR);
                APPEND_PROPERTIES_CHAIN(rayTracingProps, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR);
            )

            ENABLE_IF_AVAILABLE(VK_KHR_RAY_QUERY_EXTENSION_NAME,
                APPEND_FEATURES_CHAIN(rayTracingQueryFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR);
            )
        )

        ENABLE_IF_AVAILABLE(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,
            APPEND_FEATURES_CHAIN(fragmentShadingRateFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR);
            APPEND_PROPERTIES_CHAIN(fragmentShadingRateProps, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR);
        )

        ENABLE_IF_AVAILABLE(VK_EXT_MESH_SHADER_EXTENSION_NAME,
            APPEND_FEATURES_CHAIN(meshShaderFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT);
            APPEND_PROPERTIES_CHAIN(meshShaderProps, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT);
        )

        ENABLE_IF_AVAILABLE(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME,
            APPEND_FEATURES_CHAIN(conditionalRenderingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT);
        )

        if (vulkan::CheckExtension(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME, exts) && vulkan::CheckExtension(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME, exts) && vulkan::CheckExtension(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME, exts)) {
            enabledExts.push_back(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
            enabledExts.push_back(VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME);
            enabledExts.push_back(VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME);
        }

#if defined(__linux__)
        if (vulkan::CheckExtension(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME, exts) && vulkan::CheckExtension(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME, exts)) {
            enabledExts.push_back(VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME);
            enabledExts.push_back(VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME);
        }
#endif

        *props = nullptr;
        *features = nullptr;
        vkGetPhysicalDeviceProperties2(dev, &deviceProps2);
        return enabledExts;
    }

    VulkanInterface::VulkanInterface(void* window, Validation val, RenderDeviceTypePreference pref) {
        SH::Timer timer;

        VkResult res;

        // Vulkan supports resource aliasing by default
        capabilities |= GraphicsDeviceCapability::GENERIC_SPARSE;
        // Set the RT context
        topLevelAccelerationInstanceSize = sizeof(VkAccelerationStructureInstanceKHR);
        // Save validation mode parameter
        validation = val;
        // Set queue locks
        std::unordered_map<uint32_t, std::shared_ptr<std::mutex>> queueLocks;

        VkApplicationInfo app = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Shadow Engine",
            .applicationVersion = VK_MAKE_VERSION(1,0,0),
            .pEngineName = "Umbra",
            .engineVersion = VK_MAKE_VERSION(2,2,5),
            .apiVersion = VK_API_VERSION_1_3
        };

        uint32_t nLayers;
        res = vkEnumerateInstanceLayerProperties(&nLayers, nullptr);
        assert(res == VK_SUCCESS);
        std::vector<VkLayerProperties> layers(nLayers);
        res = vkEnumerateInstanceLayerProperties(&nLayers, layers.data());
        assert(res == VK_SUCCESS);

        uint32_t nExtensions;
        res = vkEnumerateInstanceExtensionProperties(nullptr, &nExtensions, nullptr);
        assert(res == VK_SUCCESS);
        std::vector<VkExtensionProperties> extensions(nExtensions);
        res = vkEnumerateInstanceExtensionProperties(nullptr, &nExtensions, extensions.data());
        assert(res == VK_SUCCESS);

        std::vector<const char*> instanceLayers;
        std::vector<const char*> instanceExtensions;

        // We need the KHR Surface extension
        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
        for (auto& ext : extensions) {
            // If debug utils is available, we take that
            if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
                debug = true;
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            // If Swapchain Color space extensions are available, we take that too
            if (strcmp(ext.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
                instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        }

        // TODO: Decouple this from linking against SDL, or force the renderer to include SDL.
        uint32_t nSDL;
        SDL_Vulkan_GetInstanceExtensions((SDL_Window*)window, &nSDL, nullptr);
        std::vector<const char*> sdlExts(nSDL);
        SDL_Vulkan_GetInstanceExtensions((SDL_Window*)window, &nSDL, sdlExts.data());

        // Pre-allocate so it doesn't spam reallocations
        instanceExtensions.reserve(instanceExtensions.size() + nSDL);
        for (auto& i : sdlExts)
            instanceExtensions.push_back(i);

        // Add the validation layers if we ask for them
        if (validation != Validation::DISABLED) {
            // In priority order: Khronos validation, follwoed by LunarG standard, followed by LunarG standard components, followed by LunarG core.
            static const std::vector<const char*> validationLayers[] = {
                { "VK_LAYER_KHRONOS_validation" },
                { "VK_LAYER_LUNARG_standard_validation" },
                { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects" },
                { "VK_LAYER_LUNARG_core_validation" }
            };

            for (auto& layer : validationLayers)
                if (vulkan::ValidateLayers(layer, layers)) {
                    for (auto& x : layer)
                        instanceLayers.push_back(x);
                    break;
                }
        }

        // Create the vulkan instance
        VkDebugUtilsMessengerCreateInfoEXT debugCreate = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };

        if (validation != Validation::DISABLED && debug) {
            debugCreate.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugCreate.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            if (validation == Validation::VERBOSE)
                debugCreate.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            debugCreate.pfnUserCallback = vulkan::DebugCallback;
        }

        VkInstanceCreateInfo instanceCreate = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app,
            .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
            .ppEnabledLayerNames = instanceLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
            .ppEnabledExtensionNames = instanceExtensions.data(),
            .pNext = &debugCreate
        };

        res = vkCreateInstance(&instanceCreate, nullptr, &instance);
        assert(res == VK_SUCCESS);

        if (validation != Validation::DISABLED && debug) {
            res = vkCreateDebugUtilsMessengerEXT(instance, &debugCreate, nullptr, &debugUtilsMessenger);
            assert(res == VK_SUCCESS);
        }

        // Enumerate devices, create logical device
        uint32_t nDevices;
        res = vkEnumeratePhysicalDevices(instance, &nDevices, nullptr);
        assert(res == VK_SUCCESS);

        if (nDevices == 0) {
            spdlog::error("No Vulkan devices were found. Engine cannot continue loading.");
            exit(1);
        }

        std::vector<VkPhysicalDevice> devices(nDevices);
        res = vkEnumeratePhysicalDevices(instance, &nDevices, devices.data());
        assert(res == VK_SUCCESS);

        std::vector<const char*> enabledExts;

        bool deviceIsPreferred = false;
        for (const auto& dev : devices) {
            deviceIsPreferred = false;
            enabledExts = ProcessPhysicalDevice(dev);
            if (enabledExts.empty())
                continue;

            bool isPriority =
                pref == RenderDeviceTypePreference::INTEGRATED ?
                    deviceProps2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU :
                    deviceProps2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

            // Accept devices if they're better than what we have, especially if we have nothing already.
            if (isPriority || physicalDevice == VK_NULL_HANDLE) {
                physicalDevice = dev;
                deviceIsPreferred = true;
                // Don't look for another if we reached priority.
                if (isPriority)
                    break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            spdlog::error("Unable to find a Vulkan device that supports the required extensions.");
            exit(1);
        }

        // If we have 2 integrated devices, but the preference is discrete, then we'll have saved the first but have the properties of the second, so resynchronise to the saved device.
        if (!deviceIsPreferred)
            enabledExts = ProcessPhysicalDevice(physicalDevice);

        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);
        decodeH264Profile = {
            .sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_PROFILE_INFO_KHR,
            .stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH,
            .pictureLayout = VK_VIDEO_DECODE_H264_PICTURE_LAYOUT_INTERLACED_INTERLEAVED_LINES_BIT_KHR
        };
        decodeH264Capabilities = { VK_STRUCTURE_TYPE_VIDEO_DECODE_H264_CAPABILITIES_KHR };
        capabilityH264 = {
            .profile = {
                .sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR,
                .videoCodecOperation = VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR,
                .lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
                .chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR,
                .chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR,
                .pNext = &decodeH264Profile
            },
            .decode = {
                .sType = VK_STRUCTURE_TYPE_VIDEO_DECODE_CAPABILITIES_KHR,
                .pNext = &decodeH264Capabilities
            },
            .capabilities = {
                .sType = VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR,
                .pNext = &capabilityH264.decode
            }
        };
        res = vkGetPhysicalDeviceVideoCapabilitiesKHR(physicalDevice, &capabilityH264.profile, &capabilityH264.capabilities);
        assert(res == VK_SUCCESS);

        VkFormatProperties r11g11b10Props = {};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, vulkan::convert::Format(ImageFormat::R11G11B10_FLOAT), &r11g11b10Props);

        memoryProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &memoryProps);

        // Assert the features we require in the engine
        assert(deviceProps2.properties.limits.timestampComputeAndGraphics == VK_TRUE);
        assert(deviceFeatures2.features.imageCubeArray == VK_TRUE);
        assert(deviceFeatures2.features.independentBlend == VK_TRUE);
        assert(deviceFeatures2.features.geometryShader == VK_TRUE);
        assert(deviceFeatures2.features.samplerAnisotropy == VK_TRUE);
        assert(deviceFeatures2.features.shaderClipDistance == VK_TRUE);
        assert(deviceFeatures2.features.textureCompressionBC == VK_TRUE);
        assert(deviceFeatures2.features.occlusionQueryPrecise == VK_TRUE);
        assert(deviceFeatures12.descriptorIndexing == VK_TRUE);
        assert(deviceFeatures13.dynamicRendering == VK_TRUE);

        // Update the capabilities we set
        vendorID = deviceProps2.properties.vendorID;
        deviceID = deviceProps2.properties.deviceID;
        deviceName = deviceProps2.properties.deviceName;
        driverDescription = deviceProps12.driverName;

        if (deviceProps12.driverInfo[0] != '\0')
            driverDescription += std::string(": ") + deviceProps12.driverInfo;

        type =
            deviceProps2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? RenderDeviceType::INTEGRATED :
            deviceProps2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? RenderDeviceType::DISCRETE :
            deviceProps2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? RenderDeviceType::VIRTUAL :
            deviceProps2.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? RenderDeviceType::SOFTWARE :
            RenderDeviceType::OTHER;

        if (deviceFeatures2.features.tessellationShader == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::TESSELLATION;
        if (deviceFeatures2.features.shaderStorageImageExtendedFormats == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::UAV_LOAD_FORMAT_COMMON;
        if (deviceFeatures12.shaderOutputLayer == VK_TRUE && deviceFeatures12.shaderOutputViewportIndex)
            capabilities |= GraphicsDeviceCapability::RT_VIEWPORT_WITHOUT_GEOMETRY_SHADER;
        if (rayTracingFeatures.rayTracingPipeline == VK_TRUE && rayTracingQueryFeatures.rayQuery == VK_TRUE && accelerationStructureFeatures.accelerationStructure == VK_TRUE && deviceFeatures12.bufferDeviceAddress == VK_TRUE) {
            capabilities |= GraphicsDeviceCapability::RAY_TRACING;
            shaderGroupHandleSize = rayTracingProps.shaderGroupHandleSize;
        }
        if (meshShaderFeatures.meshShader == VK_TRUE && meshShaderFeatures.taskShader == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::SUPPORTS_MESH_SHADER;
        if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::VARIABLE_RATE_SHADING;
        if (fragmentShadingRateFeatures.attachmentFragmentShadingRate == VK_TRUE) {
            capabilities |= GraphicsDeviceCapability::VARIABLE_RATE_SHADING_TIER2;
            variableRateShadingTileSize = std::min(fragmentShadingRateProps.maxFragmentShadingRateAttachmentTexelSize.width, fragmentShadingRateProps.maxFragmentShadingRateAttachmentTexelSize.height);
        }
        if (conditionalRenderingFeatures.conditionalRendering == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::PREDICATION;
        if (deviceFeatures12.samplerFilterMinmax == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::SAMPLER_MINMAX;
        if (deviceFeatures2.features.depthBounds == VK_TRUE)
            capabilities |= GraphicsDeviceCapability::DEPTH_BOUNDS_TEST;
        if (deviceFeatures2.features.sparseBinding == VK_TRUE && deviceFeatures2.features.sparseResidencyAliased == VK_TRUE) {
            if (deviceProps2.properties.sparseProperties.residencyNonResidentStrict == VK_TRUE)
                capabilities |= GraphicsDeviceCapability::SPARSE_NULL_MAPPING;
            if (deviceFeatures2.features.sparseResidencyBuffer == VK_TRUE)
                capabilities |= GraphicsDeviceCapability::SPARSE_BUFFER;
            if (deviceFeatures2.features.sparseResidencyImage2D == VK_TRUE)
                capabilities |= GraphicsDeviceCapability::SPARSE_TEXTURE2D;
            if (deviceFeatures2.features.sparseResidencyImage3D == VK_TRUE)
                capabilities |= GraphicsDeviceCapability::SPARSE_TEXTURE3D;
        }
        if ((depthStencilResolveProps.supportedDepthResolveModes & VK_RESOLVE_MODE_MIN_BIT) && (depthStencilResolveProps.supportedDepthResolveModes & VK_RESOLVE_MODE_MAX_BIT))
            capabilities |= GraphicsDeviceCapability::DEPTH_RESOLVE_MIN_MAX;
        if ((depthStencilResolveProps.supportedStencilResolveModes & VK_RESOLVE_MODE_MIN_BIT) && (depthStencilResolveProps.supportedStencilResolveModes & VK_RESOLVE_MODE_MAX_BIT))
            capabilities |= GraphicsDeviceCapability::STENCIL_RESOLVE_MIN_MAX;
        if (capabilityH264.capabilities.flags) {
            capabilities |= GraphicsDeviceCapability::VIDEO_DECODE_H264;
            videoDecodeBitstreamAlignment = std::max(std::max(videoDecodeBitstreamAlignment, capabilityH264.capabilities.minBitstreamBufferSizeAlignment), capabilityH264.capabilities.minBitstreamBufferOffsetAlignment);
        }
        if (r11g11b10Props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            capabilities |= GraphicsDeviceCapability::UAV_LOAD_FORMAT_R11G11B10_FLOAT;
        if (memoryProps.memoryProperties.memoryHeapCount == 1 && memoryProps.memoryProperties.memoryHeaps[0].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            capabilities |= GraphicsDeviceCapability::CACHE_COHERENT_UMA;

        // Setup queue families
        uint32_t nFamilies = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &nFamilies, 0);
        queueFamilies.resize(nFamilies);
        queueFamiliesVideo.resize(nFamilies);
        // Link the family to the video family
        for (uint32_t i = 0; i < nFamilies; i++) {
            queueFamilies[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            queueFamilies[i].pNext = &queueFamiliesVideo[i];
            queueFamiliesVideo[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
        }
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &nFamilies, queueFamilies.data());

        // Figure out what these queues can do
        for (uint32_t queue = 0; queue < nFamilies; queue++) {
            auto& family = queueFamilies[queue];
            auto& video = queueFamiliesVideo[queue];

            // Find the first render-capable queue
            if (graphicsFamily == VK_QUEUE_FAMILY_IGNORED && family.queueFamilyProperties.queueCount > 0 && family.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsFamily = queue;
                // Check whether this queue supports sparse binding.
                if (family.queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    queues[QueueType::GRAPHICS].sparse = true;
            }

            // Find the first copy capable queue
            if (copyFamily == VK_QUEUE_FAMILY_IGNORED && family.queueFamilyProperties.queueCount > 0 && family.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT)
                copyFamily = queue;

            // Find the first compute capable queue
            if (computeFamily == VK_QUEUE_FAMILY_IGNORED && family.queueFamilyProperties.queueCount > 0 && family.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
                computeFamily = queue;

            // Find a dedicated video queue if possible
            if (videoFamily == VK_QUEUE_FAMILY_IGNORED && family.queueFamilyProperties.queueCount > 0 && (family.queueFamilyProperties.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) && (video.videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR))
                videoFamily = queue;

            // Find a dedicated copy queue
            if (family.queueFamilyProperties.queueCount > 0 && family.queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT && !(family.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(family.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                copyFamily = queue;
                if (family.queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    queues[QueueType::COPY].sparse = true;
            }

            // Find a dedicated compute bit
            if (family.queueFamilyProperties.queueCount > 0 && family.queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT && !(family.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                computeFamily = queue;
                if (family.queueFamilyProperties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
                    queues[QueueType::COMPUTE].sparse = true;
            }
        }

        // Iterate again so this doesn't get overwritten by "Find a dedicated copy queue" above.
        for (uint32_t queue = 0; queue < nFamilies; queue++) {
            if (queueFamilies[queue].queueFamilyProperties.queueCount > 0 && queueFamilies[queue].queueFamilyProperties.queueFlags == (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)) {
                copyFamily = queue;
                queues[QueueType::COPY].sparse = true;
            }
        }

        // Prepare creation of the queues
        std::vector<VkDeviceQueueCreateInfo> queueCreate;
        std::unordered_set<uint32_t> uniqueFamilies = { static_cast<uint32_t>(graphicsFamily), static_cast<uint32_t>(copyFamily), static_cast<uint32_t>(computeFamily) };
        if (videoFamily != VK_QUEUE_FAMILY_IGNORED)
            uniqueFamilies.insert(videoFamily);

        float priority = 1.0;
        for (uint32_t queue : uniqueFamilies) {
            queueLocks.emplace(queue, std::make_shared<std::mutex>());
            queueCreate.emplace_back(VkDeviceQueueCreateInfo { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, queue, 1, &priority });
            families.push_back(queue);
        }

        // Create logical device
        VkDeviceCreateInfo deviceCreate = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreate.size()),
            .pQueueCreateInfos = queueCreate.data(),
            .pEnabledFeatures = nullptr,
            .pNext = &deviceFeatures2,
            .enabledExtensionCount = static_cast<uint32_t>(enabledExts.size()),
            .ppEnabledExtensionNames = enabledExts.data()
        };

        res = vkCreateDevice(physicalDevice, &deviceCreate, nullptr, &device);
        assert(res == VK_SUCCESS);

        // Fetch queues from the device
        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, computeFamily, 0, &computeQueue);
        vkGetDeviceQueue(device, copyFamily, 0, &copyQueue);
        if (videoFamily != VK_QUEUE_FAMILY_IGNORED)
            vkGetDeviceQueue(device, videoFamily, 0, &videoQueue);

        queues[QueueType::GRAPHICS].queue = graphicsQueue;
        queues[QueueType::GRAPHICS].locker = queueLocks[graphicsFamily];
        queues[QueueType::COMPUTE].queue = computeQueue;
        queues[QueueType::COMPUTE].locker = queueLocks[computeFamily];
        queues[QueueType::COPY].queue = copyQueue;
        queues[QueueType::COPY].locker = queueLocks[copyFamily];
        if (videoFamily != VK_QUEUE_FAMILY_IGNORED) {
            queues[QueueType::VIDEO_DECODE].queue = videoQueue;
            queues[QueueType::VIDEO_DECODE].locker = queueLocks[videoFamily];
        }

        // Prepare the memory manager
        memoryManager = std::make_shared<MemoryManager>();
        memoryManager->device = device;
        memoryManager->instance = instance;

        // Initialize VMA
        VmaAllocatorCreateInfo allocatorCreate = {
            .physicalDevice = physicalDevice,
            .device = device,
            .instance = instance,
            .flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT
        };
        if (deviceFeatures12.bufferDeviceAddress)
            allocatorCreate.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

        res = vmaCreateAllocator(&allocatorCreate, &memoryManager->allocator);
        assert(res == VK_SUCCESS);

#if defined(__linux__)
        std::vector<VkExternalMemoryHandleTypeFlags> externalMemory;
        externalMemory.resize(memoryProps2.memoryProperties.memoryTypeCount, VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
        allocatorCreate.pTypeExternalMemoryHandleTypes = externalMemory.data();
#endif

        res = vmaCreateAllocator(&allocatorCreate, &memoryManager->externalAllocator);
        assert(res == VK_SUCCESS);

        // Initialize the uploader
        upload.Init(this);

        // Create fences
        for (uint32_t frame = 0; frame < frameCount; frameCount++) {
            for (int queue = 0; queue < QueueType::COUNT; queue++) {
                if (queues[queue].queue == VK_NULL_HANDLE) continue;
                VkFenceCreateInfo fenceCreate = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
                res = vkCreateFence(device, &fenceCreate, nullptr, &frameFence[frame][queue]);
                assert(res == VK_SUCCESS);
            }
        }

        // Create the null resources
        VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 4,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .flags = 0
        };

        VmaAllocationCreateInfo allocationInfo = {
            .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        };

        res = vmaCreateBuffer(memoryManager->allocator, &bufferInfo, &allocationInfo, &nullBuffer, &nullAllocation, nullptr);
        assert(res == VK_SUCCESS);

        VkBufferViewCreateInfo bufferViewInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .range = VK_WHOLE_SIZE,
            .buffer = nullBuffer
        };
        res = vkCreateBufferView(device, &bufferViewInfo, nullptr, &nullBufferView);

        VkImageCreateInfo imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .extent = { 1, 1, 1 },
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .arrayLayers = 1,
            .mipLevels = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            .flags = 0
        };

        allocationInfo = { .usage = VMA_MEMORY_USAGE_GPU_ONLY };

        imageInfo.imageType = VK_IMAGE_TYPE_1D;
        res = vmaCreateImage(memoryManager->allocator, &imageInfo, &allocationInfo, &nullImage1, &nullAllocation, nullptr);
        assert(res == VK_SUCCESS);

        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.arrayLayers = 6;
        res = vmaCreateImage(memoryManager->allocator, &imageInfo, &allocationInfo, &nullImage2,  &nullAllocation, nullptr);
        assert(res == VK_SUCCESS);

        imageInfo.imageType = VK_IMAGE_TYPE_3D;
        imageInfo.flags = 0;
        imageInfo.arrayLayers = 1;
        res = vmaCreateImage(memoryManager->allocator, &imageInfo, &allocationInfo, &nullImage3, &nullAllocation, nullptr);
        assert(res == VK_SUCCESS);

        // Transition the images and buffers so that the driver doesn't immediately error when we try to use them
        // (the null resources are meant to be USABLE but not VALID.)
        Uploader::Copy command = upload.Allocate(0);
        VkImageMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .oldLayout = imageInfo.initialLayout,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseArrayLayer = 0,
                .baseMipLevel = 0,
                .levelCount = 1,
                .layerCount = 1
            },
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = nullImage1
        };
        VkDependencyInfo dependency = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(command.transitionBuffer, &dependency);
        barrier.image = nullImage3;
        vkCmdPipelineBarrier2(command.transitionBuffer, &dependency);
        barrier.image = nullImage2;
        barrier.subresourceRange.layerCount = 6;
        vkCmdPipelineBarrier2(command.transitionBuffer, &dependency);

        // Submit the transition bufffer immediately
        upload.Submit(command);

        // Create image views
        VkImageViewCreateInfo view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseArrayLayer = 0,
                .layerCount = 1,
                .baseMipLevel = 0,
                .levelCount = 1
            },
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .image = nullImage1,
            .viewType = VK_IMAGE_VIEW_TYPE_1D
        };

        res = vkCreateImageView(device, &view, nullptr, &nullImageView1);
        assert(res == VK_SUCCESS);
        view.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        res = vkCreateImageView(device, &view, nullptr, &nullImageView1A);
        assert(res == VK_SUCCESS);

        view.image = nullImage2;
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        res = vkCreateImageView(device, &view, nullptr, &nullImageView2);
        assert(res == VK_SUCCESS);
        view.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        res = vkCreateImageView(device, &view, nullptr, &nullImageView2A);
        assert(res == VK_SUCCESS);
        view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        view.subresourceRange.layerCount = 6;
        res = vkCreateImageView(device, &view, nullptr, &nullImageViewC);
        assert(res == VK_SUCCESS);
        view.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        res = vkCreateImageView(device, &view, nullptr, &nullImageViewCA);
        assert(res == VK_SUCCESS);

        view.image = nullImage3;
        view.subresourceRange.layerCount = 1;
        view.viewType = VK_IMAGE_VIEW_TYPE_3D;
        res = vkCreateImageView(device, &view, nullptr, &nullImageView3);
        assert(res == VK_SUCCESS);

        VkSamplerCreateInfo sampler = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
        res = vkCreateSampler(device, &sampler, nullptr, &nullSampler);

        // Finished creating null resources, set some engine state now.
        timestampFrequency = size_t(1 / double(deviceProps2.properties.limits.timestampPeriod) * 1000'000'000);

        psoDynamicState.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
        psoDynamicState.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
        psoDynamicState.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
        psoDynamicState.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
        if (CheckCapability(GraphicsDeviceCapability::DEPTH_BOUNDS_TEST))
            psoDynamicState.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
        if (CheckCapability(GraphicsDeviceCapability::VARIABLE_RATE_SHADING))
            psoDynamicState.push_back(VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR);
        // Vertex input binding state last so we can exclude it later
        psoDynamicState.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);

        dynamicStateCreate = VkPipelineDynamicStateCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(psoDynamicState.size()),
            .pDynamicStates = psoDynamicState.data()
        };

        dynamicStateMeshCreate = VkPipelineDynamicStateCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(psoDynamicState.size() - 1), // No vertex input bindings in mesh shaders.
            .pDynamicStates = psoDynamicState.data()
        };

        // Initialize bindless descriptors
        const uint32_t maxBindless = 100'000;

        if (deviceFeatures12.descriptorBindingSampledImageUpdateAfterBind == VK_TRUE) {
            memoryManager->bindlessSamplers.Init(device, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 256);
            memoryManager->bindlessImages.Init(device, VK_DESCRIPTOR_TYPE_SAMPLER, std::min(maxBindless, deviceProps12.maxDescriptorSetUpdateAfterBindSampledImages / 4));
        }
        if (deviceFeatures12.descriptorBindingUniformTexelBufferUpdateAfterBind == VK_TRUE)
            memoryManager->bindlessUniformTBuffers.Init(device, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, std::min(maxBindless, deviceProps12.maxDescriptorSetUpdateAfterBindSampledImages / 4));
        if (deviceFeatures12.descriptorBindingStorageBufferUpdateAfterBind == VK_TRUE)
            memoryManager->bindlessStorageBuffers.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min(maxBindless, deviceProps12.maxDescriptorSetUpdateAfterBindStorageBuffers / 4));
        if (deviceFeatures12.descriptorBindingStorageImageUpdateAfterBind == VK_TRUE)
            memoryManager->bindlessStorageImages.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, std::min(maxBindless, deviceProps12.maxDescriptorSetUpdateAfterBindStorageImages / 4));
        if (deviceFeatures12.descriptorBindingStorageTexelBufferUpdateAfterBind == VK_TRUE)
            memoryManager->bindlessStorageTBuffers.Init(device, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, std::min(maxBindless, deviceProps12.maxDescriptorSetUpdateAfterBindStorageImages / 4));
        if (CheckCapability(GraphicsDeviceCapability::RAY_TRACING))
            memoryManager->bindlessRT.Init(device, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 32);

        // Load the pipeline cache from disk.
        ShadowEngine::FileInput cacheFile;
        std::vector<uint8_t> cacheData;
        cacheFile.open(std::string("./cache/PipelineCache-Vulkan.cache"));
        cacheData.resize(cacheFile.size());
        cacheFile.read(cacheData.data(), cacheFile.size());

        // Verify cache file integrity if it contains data
        if (!cacheData.empty()) {
            uint32_t headerLength = 0, cacheHeaderVersion = 0, vendorID = 0, deviceID = 0;
            uint8_t cacheUUID[VK_UUID_SIZE] = {};

            std::memcpy(&headerLength, cacheData.data(), 4);
            std::memcpy(&cacheHeaderVersion, cacheData.data() + 4, 4);
            std::memcpy(&vendorID, cacheData.data() + 8, 4);
            std::memcpy(&deviceID, cacheData.data() + 12, 4);
            std::memcpy(cacheUUID, cacheData.data() + 16, VK_UUID_SIZE);

            bool bad = false;
            if (headerLength <= 0) bad = true;
            if (cacheHeaderVersion != VK_PIPELINE_CACHE_HEADER_VERSION_ONE) bad = true;
            if (vendorID != deviceProps2.properties.vendorID) bad = true;
            if (deviceID != deviceProps2.properties.deviceID) bad = true;
            if (memcmp(cacheUUID, deviceProps2.properties.pipelineCacheUUID, sizeof(cacheUUID)) != 0) bad = true;

            // Don't bother continuing with bad data
            if (bad)
                cacheData.clear();
        }

        cacheFile.close();

        VkPipelineCacheCreateInfo cacheCreate = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .initialDataSize = cacheData.size(),
            .pInitialData = cacheData.data()
        };

        res = vkCreatePipelineCache(device, &cacheCreate, nullptr, &pipelineCache);
        assert(res == VK_SUCCESS);

        // Create static samplers
        VkSamplerCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.compareEnable = false;
        createInfo.compareOp = VK_COMPARE_OP_NEVER;
        createInfo.minLod = 0;
        createInfo.maxLod = FLT_MAX;
        createInfo.mipLodBias = 0;
        createInfo.anisotropyEnable = false;
        createInfo.maxAnisotropy = 0;

        // sampler_linear_clamp:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_linear_wrap:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        //sampler_linear_mirror:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_point_clamp:
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_point_wrap:
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_point_mirror:
        createInfo.minFilter = VK_FILTER_NEAREST;
        createInfo.magFilter = VK_FILTER_NEAREST;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_aniso_clamp:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.anisotropyEnable = true;
        createInfo.maxAnisotropy = 16;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_aniso_wrap:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        createInfo.anisotropyEnable = true;
        createInfo.maxAnisotropy = 16;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_aniso_mirror:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        createInfo.anisotropyEnable = true;
        createInfo.maxAnisotropy = 16;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        // sampler_cmp_depth:
        createInfo.minFilter = VK_FILTER_LINEAR;
        createInfo.magFilter = VK_FILTER_LINEAR;
        createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.anisotropyEnable = false;
        createInfo.maxAnisotropy = 0;
        createInfo.compareEnable = true;
        createInfo.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
        createInfo.minLod = 0;
        createInfo.maxLod = 0;
        res = vkCreateSampler(device, &createInfo, nullptr, &immutableSamplers.emplace_back());
        assert(res == VK_SUCCESS);

        spdlog::info("Initialized rx::vulkan in " + std::to_string(std::round(timer.elapsedMillis())) + "ms");
    }

    VulkanInterface::~VulkanInterface() {
        VkResult res = vkDeviceWaitIdle(device);
        assert(res == VK_SUCCESS);

        for (uint32_t frame = 0; frame < frameCount; frame++)
            for (int queue = 0; queue < QueueType::COUNT; queue++)
                vkDestroyFence(device, frameFence[frame][queue], nullptr);
        upload.Destroy();

        for (auto& x : PSOcache) {
            vkDestroyPipelineLayout(device, x.second.layout, nullptr);
            vkDestroyDescriptorSetLayout(device, x.second.descriptorLayout, nullptr);
        }

        for (auto& command : cmds) {
            for (int frame = 0; frame < frameCount; frame++)
                for (int queue = 0; queue < QueueType::COUNT; queue++)
                    vkDestroyCommandPool(device, command->pools[frame][queue], nullptr);
            for (auto& x : command->pipelines)
                vkDestroyPipeline(device, x.second, nullptr);
            for (auto& x : command->bindPools)
                x.Destroy();
        }

        for (auto& x : pipelines)
            vkDestroyPipeline(device, x.second, nullptr);

        for (auto& x : signalPool)
            vkDestroySemaphore(device, x, nullptr);

        vmaDestroyBuffer(memoryManager->allocator, nullBuffer, nullAllocation);
        vkDestroyBufferView(device, nullBufferView, nullptr);
        vmaDestroyImage(memoryManager->allocator, nullImage1, nullAllocation);
        vmaDestroyImage(memoryManager->allocator, nullImage2, nullAllocation);
        vmaDestroyImage(memoryManager->allocator, nullImage3, nullAllocation);
        vkDestroyImageView(device, nullImageView1, nullptr);
        vkDestroyImageView(device, nullImageView1A, nullptr);
        vkDestroyImageView(device, nullImageView2, nullptr);
        vkDestroyImageView(device, nullImageView2A, nullptr);
        vkDestroyImageView(device, nullImageView3, nullptr);
        vkDestroyImageView(device, nullImageViewC, nullptr);
        vkDestroyImageView(device, nullImageViewCA, nullptr);
        vkDestroySampler(device, nullSampler, nullptr);

        for (VkSampler s : immutableSamplers)
            vkDestroySampler(device, s, nullptr);

        if (pipelineCache != VK_NULL_HANDLE) {
            size_t size;
            res = vkGetPipelineCacheData(device, pipelineCache, &size, nullptr);
            assert(res == VK_SUCCESS);
            std::vector<uint8_t> data(size);
            res = vkGetPipelineCacheData(device, pipelineCache, &size, data.data());
            assert(res == VK_SUCCESS);

            ShadowEngine::FileOutput file;
            file.open("./cache/PipelineCache-Vulkan.cache");
            file.write(data.data(), data.size());
            file.close();

            vkDestroyPipelineCache(device, pipelineCache, nullptr);
        }

        if (debugUtilsMessenger != VK_NULL_HANDLE)
            vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
    }

}
