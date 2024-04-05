#include <renderer/helper/TextureHelper.h>
#include "renderer/Interface.h"

namespace rx::Helper::Texture {

  bool CreateTexture( rx::Texture& tex, const uint8_t* data, uint32_t width, uint32_t height, rx::ImageFormat format, rx::Swizzle swizzle) {
      if (data == nullptr)
          return false;

      rx::Interface* iface = rx::GetInterface();

      rx::TextureMeta meta {
          .width = width,
          .height = height,
          .arraySize = 1,
          .mipLevels = 1,
          .format = format,
          .sampleCount = 1,
          .bindFlag = BindFlag::SHADER_RESOURCE,
          .swizzle = swizzle
      };

      SubresourceMeta subMeta {
          .data = data,
          .rowPitch = width * GetFormatStride(format) / GetFormatBlockSize(format)
      };

      return iface->CreateTexture(&meta, &subMeta, &tex);
  }
}