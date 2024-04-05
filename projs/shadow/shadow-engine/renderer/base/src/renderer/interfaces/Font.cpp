#include <renderer/interfaces/Font.h>
#include <unordered_set>
#include "imstb_truetype.h"
#include "spdlog/spdlog.h"
#include "shadow/assets/fs/iostream.h"
#include "shadow/core/ShadowApplication.h"
#include "renderer/interfaces/Sprite.h"
#include "shadow/util/string-helpers.h"
#include "renderer/Renderer.h"
#include "shadow/core/Time.h"
#include <shader/modules/Font.h>
#include <renderer/assets/BuiltinFont.h>
#include <renderer/helper/TextureHelper.h>

namespace rx::Font {
  namespace internal {
    static BlendState blendState;
    static RasterizerState rasterizerState;
    static DepthStencilState depthStencilState;
    static DepthStencilState depthStencilState_depthTest;

    static Shader vertexShader;
    static Shader pixelShader;
    static PipelineState PSO;
    static PipelineState PSO_depthTest;

    static thread_local rx::Canvas canvas;

    static rx::Texture texture;

    static std::vector<std::unique_ptr<FontStyle>> styles;

    struct Glyph {
      float x;
      float y;
      float width;
      float height;
      float tc_left;
      float tc_right;
      float tc_top;
      float tc_bottom;
      const FontStyle *style = nullptr;
    };

    static std::unordered_map<int32_t, Glyph> glyphs;
    static std::unordered_map<int32_t, rx::Sprite::Packer::Rect> rects;

    struct Bitmap {
      int width;
      int height;
      int xOffset;
      int yOffset;
      std::vector<uint8_t> data;
    };

    static std::unordered_map<int32_t, Bitmap> bitmaps;

    // Signed Distance Field - smoothing for fonts
    namespace SDF {
      static constexpr int padding = 5;
      static constexpr unsigned char edgeVal = 180;
      static constexpr float pixelDistScale = float(edgeVal) / float(padding);
    }

    constexpr int32_t GlyphHash(int charCode, bool sdf, int style, int height) {
        return ((charCode & 0xFFFF) << 16) | (int(sdf) << 15) | ((style & 0x1F) << 10) | (height & 0x3FF);
    }
    constexpr int GlyphHashCharCode(int32_t hash) { return int((hash >> 16) & 0xFFFF); }
    constexpr bool GlyphHashSDF(int32_t hash) { return bool((hash >> 15) & 0x1); }
    constexpr int GlyphHashStyle(int32_t hash) { return int((hash >> 10) & 0x1F); }
    constexpr int GlyphHashHeight(int32_t hash) { return int((hash >> 0) & 0x3FF); }

    static std::unordered_set<int32_t> pendingGlyphs;
    static std::mutex glyphLock;

    struct ParseStatus {
      Font::Cursor cursor;
      uint32_t quads = 0;
      size_t lastBegin = 0;
      bool startNew = false;
    };

    static thread_local std::vector<FontVertex> vertexes;

    ParseStatus ParseText(const wchar_t *text, size_t len, const Font::RenderMode &params) {
        ParseStatus status;
        status.cursor = params.cursor;

        vertexes.clear();

        // Spaces are 25% the width of a single character (plus padding)
        const float whitespaceSize = (float(params.size) + params.spacingX) * 0.25f;
        // Tabs are 4 spaces.
        const float tabSize = whitespaceSize * 4;
        // Vertical lines are the height of a character + the vertical spacing apart
        const float newlineSize = (float(params.size) + params.spacingY);

        // There are three things that can cause a word-wrap to happen mid-line, and it needs to happen at the end of every line.
        // For brevity, here's the chunk of code that would otherwise be repeated 5 times.
        // "consider" because we may not need to actually do the wrapping (ie. run this at every space, so we can break at the end of a line)
        auto considerWrap = [&] {
          // Whether newline, tab or space, we can start a new line here.
          status.startNew = true;
          if (status.lastBegin > 0 && params.hWrap >= 0 && status.cursor.pos.x >= params.hWrap - 1) {
              float wordOffset = vertexes[status.lastBegin].pos.x + whitespaceSize;
              for (size_t i = status.lastBegin; i < status.quads * 4; ++i) {
                  vertexes[i].pos.x -= wordOffset;
                  vertexes[i].pos.y += newlineSize;
              }

              status.cursor.pos.x -= wordOffset;
              status.cursor.pos.y += newlineSize;
              status.cursor.size.x = std::max(status.cursor.size.x, status.cursor.pos.x);
              status.cursor.size.y = std::max(status.cursor.size.y, status.cursor.pos.y + newlineSize);
          }
        };

        status.cursor.size.y = status.cursor.pos.y + newlineSize;
        for (size_t i = 0; i < len; i++) {
            int codepoint = (int) text[i];
            const int32_t hash = GlyphHash(codepoint, params.isSDFEnabled(), params.style, params.size);

            if (glyphs.count(hash) == 0) {
                std::scoped_lock locker(glyphLock);
                pendingGlyphs.insert(hash);
                continue;
            }

            if (codepoint == '\n') {
                considerWrap();
                status.cursor.pos.x = 0;
                status.cursor.pos.y += newlineSize;
            } else if (codepoint == ' ') {
                considerWrap();
                status.cursor.pos.x += whitespaceSize;
            } else if (codepoint == '\t') {
                considerWrap();
                status.cursor.pos.x += tabSize;
            } else {
                // Normal non-space character
                const Glyph &gl = glyphs.at(hash);
                const float gWidth = gl.width;
                const float gHeight = gl.height;
                const float gX = gl.x;
                const float gY = gl.y;
                const float fontScale = stbtt_ScaleForPixelHeight(&gl.style->fontInfo, (float) params.size);

                const size_t vertexID = size_t(status.quads) * 4;
                vertexes.resize(vertexID + 4);
                status.quads++;

                if (status.startNew)
                    status.lastBegin = vertexID;
                status.startNew = false;

                const float left = status.cursor.pos.x + gX;
                const float right = left + gWidth;
                const float top = status.cursor.pos.y + gY;
                const float bottom = top + gHeight;

                vertexes[vertexID + 0].pos = float2(left, top);
                vertexes[vertexID + 1].pos = float2(right, top);
                vertexes[vertexID + 2].pos = float2(left, bottom);
                vertexes[vertexID + 3].pos = float2(right, bottom);

                vertexes[vertexID + 0].uv = float2(left, top);
                vertexes[vertexID + 1].uv = float2(right, top);
                vertexes[vertexID + 2].uv = float2(left, bottom);
                vertexes[vertexID + 3].uv = float2(right, bottom);

                int advance = 0, lsb;
                stbtt_GetCodepointHMetrics(&gl.style->fontInfo, codepoint, &advance, &lsb);
                status.cursor.pos.x += advance * fontScale;
                status.cursor.pos.x += params.spacingX;

                // Process font-specific kerning.
                if (len > 1 && i < len - 1 && text[i + 1]) {
                    int codeNext = (int) text[i + 1];
                    int kern = stbtt_GetCodepointKernAdvance(&gl.style->fontInfo, codepoint, codeNext);
                    status.cursor.pos.x += kern * fontScale;
                }
            }

            status.cursor.size.x = std::max(status.cursor.size.x, status.cursor.pos.x);
            status.cursor.size.y = std::max(status.cursor.size.y, status.cursor.pos.y + newlineSize);
        }

        // We might have bumped the end of the line and not hit a character that would make us think about it, so check now before we start again.
        considerWrap();

        return status;
    }

    static thread_local std::string tempCharBuffer;
    static thread_local std::wstring tempWcharBuffer;
    ParseStatus ParseText(const char *text, size_t len, const Font::RenderMode &params) {
        tempCharBuffer = text;
        SH::Util::Str::StringConvert(tempCharBuffer, tempWcharBuffer);
        return ParseText(tempWcharBuffer.c_str(), tempWcharBuffer.length(), params);
    }

    void CommitText(void *vertexGPU) {
        std::memcpy(vertexGPU, vertexes.data(), sizeof(FontVertex) * vertexes.size());
    }
  }

  void LoadShaders() {
      rx::Renderer::LoadShader(rx::ShaderStage::VS, internal::vertexShader, "fontVS.cso");
      rx::Renderer::LoadShader(rx::ShaderStage::FS, internal::pixelShader, "fontPS.cso");

      rx::PipelineStateMeta meta{
          .vertex = &internal::vertexShader,
          .fragment = &internal::pixelShader,
          .blend = &internal::blendState,
          .rasterizer = &internal::rasterizerState,
          .depthStencil = &internal::depthStencilState,
          .topology = rx::Topology::TRIANGLESTRIP
      };
      rx::GetInterface()->CreatePipelineState(&meta, &internal::PSO);

      meta.depthStencil = &internal::depthStencilState_depthTest;
      rx::GetInterface()->CreatePipelineState(&meta, &internal::PSO_depthTest);
  }

  void Initialize() {
      SH::Timer timer;

      if (internal::styles.empty())
          AddFont("Liberation Sans", liberation_sans, sizeof(liberation_sans));

      RasterizerState raster{
          .fillMode = DrawMode::SOLID,
          .culling = CullingMode::NONE,
          .reverseWindingOrder = true,
          .depthBias = 0,
          .depthBiasClamp = 0,
          .slopeDepthBias = 0,
          .depthClipping = false,
          .multisample = false,
          .antialiasing = false
      };
      internal::rasterizerState = raster;

      BlendState blend{
          .independent = false
      };
      blend.targets[0] = {
          .enable = true,
          .source = BlendData::SOURCE_ALPHA,
          .dest = BlendData::SOURCE_ALPHA_INVERSE,
          .op = BlendOp::ADD,
          .sourceAlpha = BlendData::ONE,
          .destAlpha = BlendData::SOURCE_ALPHA_INVERSE,
          .opAlpha = BlendOp::ADD,
          .writeMask = ColorWrite::ENABLE_ALL
      };
      internal::blendState = blend;

      DepthStencilState depth{
          .depth = false,
          .stencil = false
      };
      internal::depthStencilState = depth;

      depth = {
          .depth = true,
          .mask = DepthWriteMask::UNMASKED,
          .comparison = ComparisonFunc::GREATER
      };
      internal::depthStencilState_depthTest = depth;

      // EVENT HANDLER: RELOAD SHADERS
      LoadShaders();

      spdlog::info("Font Subsystem initialized in " + std::to_string(int(std::round(timer.elapsed()))) + " ms.");
  }

  void InvalidateAtlas() {
      internal::texture = {};
      internal::glyphs.clear();
      internal::rects.clear();
      internal::bitmaps.clear();
  }

  void UpdateAtlas(float scaling) {
      std::scoped_lock lock(internal::glyphLock);

      static float upscalePrev = 1;
      const float upscale = 1.0f / scaling;

      // DPI probably changed. Reupload atlas.
      if (upscalePrev != scaling) {
          InvalidateAtlas();
          upscalePrev = scaling;
      }

      if (!internal::pendingGlyphs.empty()) {
          for (int32_t hash : internal::pendingGlyphs) {
              const int codepoint = internal::GlyphHashCharCode(hash);
              bool SDF = internal::GlyphHashSDF(hash);
              int style = internal::GlyphHashStyle(hash);
              const float height = internal::GlyphHashHeight(hash);

              FontStyle *font = internal::styles[style].get();
              int glyphIndex = stbtt_FindGlyphIndex(&font->fontInfo, codepoint);
              // Font doesn't have this character. Find one that does.
              if (glyphIndex == 0) {
                  style = 0;
                  while (glyphIndex == 0 && style < internal::styles.size()) {
                      font = internal::styles[style].get();
                      glyphIndex = stbtt_FindGlyphIndex(&font->fontInfo, codepoint);
                      style++;
                  }
              }

              float fontScaling = stbtt_ScaleForPixelHeight(&font->fontInfo, height * scaling);
              internal::Bitmap &bmp = internal::bitmaps[hash];
              bmp.width = 0;
              bmp.height = 0;
              bmp.xOffset = 0;
              bmp.yOffset = 0;

              if (SDF) {
                  unsigned char *data = stbtt_GetGlyphSDF(&font->fontInfo,
                                                          fontScaling,
                                                          glyphIndex,
                                                          SDF::padding,
                                                          SDF::edgeValue,
                                                          SDF::pixelDistanceScale,
                                                          &bmp.width,
                                                          &bmp.height,
                                                          &bmp.xOffset,
                                                          &bmp.yOffset);
                  bmp.data.resize(bmp.width * bmp.height);
                  std::memcpy(bmp.data.data(), data, bmp.data.size());
                  stbtt_FreeSDF(data, nullptr);
              } else {
                  unsigned char *data = stbtt_GetGlyphBitmap(&font->fontInfo,
                                                             fontScaling,
                                                             fontScaling,
                                                             glyphIndex,
                                                             &bmp.width,
                                                             &bmp.height,
                                                             &bmp.xOffset,
                                                             &bmp.yOffset);
                  bmp.data.resize(bmp.width * bmp.height);
                  std::memcpy(bmp.data.data(), data, bmp.data.size());
                  stbtt_FreeBitmap(data, nullptr);
              }

              rx::Sprite::Packer::Rect rect = {
                  .id = hash,
                  .w = bmp.width + 2,
                  .h = bmp.height + 2
              };
              internal::rects[hash] = rect;

              internal::Glyph &glyph = internal::glyphs[hash];
              glyph.x = float(bmp.xOffset) * upscale;
              glyph.y = (float(bmp.yOffset) + float(font->asc) * fontScaling) * upscale;
              glyph.width = float(bmp.width) * upscale;
              glyph.height = float(bmp.height) * upscale;
              glyph.style = font;
          }
          internal::pendingGlyphs.clear();

          // Repack.
          static thread_local rx::Sprite::Packer packer;
          packer.Clear();
          for (auto &it : internal::rects)
              packer.AddRect(it.second);

          if (packer.Pack(4096)) {
              const int atlasWidth = packer.width;
              const int atlasHeight = packer.height;
              const float invWidth = 1.0f / atlasWidth;
              const float invHeight = 1.0f / atlasHeight;

              std::vector<uint8_t> atlas(size_t(atlasWidth) * size_t(atlasHeight));
              std::fill(atlas.begin(), atlas.end(), 0);

              for (auto &rect : packer.rects) {
                  rect.x += 1;
                  rect.y += 1;
                  rect.w -= 2;
                  rect.h -= 2;

                  const int32_t hash = rect.id;
                  internal::Glyph &g = internal::glyphs[hash];
                  internal::Bitmap &bmp = internal::bitmaps[hash];

                  // Copy data row by row
                  for (int row = 0; row < bmp.height; row++) {
                      uint8_t *dst = atlas.data() + rect.x + static_cast<ptrdiff_t>((rect.y + row)) * atlasWidth;
                      uint8_t *src = bmp.data.data() + row * bmp.width;
                      std::memcpy(dst, src, bmp.width);
                  }

                  g.tc_left = float(rect.x);
                  g.tc_right = g.tc_left + float(rect.w);
                  g.tc_top = float(rect.y);
                  g.tc_bottom = g.tc_top + float(rect.h);

                  g.tc_left *= invWidth;
                  g.tc_right *= invWidth;
                  g.tc_top *= invHeight;
                  g.tc_bottom *= invHeight;
              }

              rx::Helper::Texture::CreateTexture(internal::texture,
                                                 atlas.data(),
                                                 atlasWidth,
                                                 atlasHeight,
                                                 ImageFormat::R8_UNORM);
              rx::GetInterface()->SetName(&internal::texture, "rx::Font::atlas");
          } else {
              assert(0);
          }
      }
  }

  const rx::Texture *GetAtlas() {
      return &internal::texture;
  }

  int AddFont(const std::string &name) {
      // Return font if it already exists
      for (size_t i = 0; i < internal::styles.size(); i++) {
          const FontStyle &style = *internal::styles[i];
          if (style.name.compare(name) == 0)
              return int(i);
      }

      internal::styles.push_back(std::make_unique<FontStyle>());
      internal::styles.back()->Create(name);
      InvalidateAtlas();
      return int(internal::styles.size() - 1);
  }

  int AddFont(const std::string &name, const uint8_t *data, size_t size) {
      // Return font if it already exists
      for (size_t i = 0; i < internal::styles.size(); i++) {
          const FontStyle &style = *internal::styles[i];
          if (style.name.compare(name) == 0)
              return int(i);
      }

      internal::styles.push_back(std::make_unique<FontStyle>());
      internal::styles.back()->Create(name, data, size);
      InvalidateAtlas();
      return int(internal::styles.size() - 1);
  }

  template<typename T>
  concept Character =
  std::same_as<T, char> ||
      std::same_as<T, wchar_t>;

  template<typename T>
  requires Character<T>
  Cursor DrawInternal(const T *text, size_t len, const Font::RenderMode &params, ThreadCommands cmd) {
      if (len <= 0)
          return {};

      auto parse = internal::ParseText(text, len, params);

      if (parse.quads > 0) {
          rx::Interface *iface = rx::GetInterface();
          rx::Interface::GPUAllocation mem = iface->CreateStagingBuffer(sizeof(FontVertex) * parse.quads * 4, cmd);
          if (!mem.isValid())
              return parse.cursor;
          internal::CommitText(mem.data);

          FontConstants font = {
              .bufferIndex = iface->GetDescriptorIdx(&mem.buffer, ImageViewType::SHADER_RESOURCE),
              .bufferOffset = (uint32_t) mem.offset,
              .textureIndex = iface->GetDescriptorIdx(&internal::texture, ImageViewType::SHADER_RESOURCE)
          };

          if (font.bufferIndex < 0 || font.textureIndex < 0)
              return parse.cursor;

          iface->EventBegin("Font Render", cmd);

          if (params.isDepthTestEnabled())
              iface->BindPSO(&internal::PSO_depthTest, cmd);
          else
              iface->BindPSO(&internal::PSO, cmd);

          font.flags = 0;
          if (params.isSDFEnabled())
              font.flags |= FONT_FLAG_SDF_RENDERING;
          if (params.isHDR10Enabled())
              font.flags |= FONT_FLAG_OUTPUT_HDR10;
          if (params.isLinearEnabled()) {
              font.flags |= FONT_FLAG_OUTPUT_LINEAR;
              font.hdrScale = params.hdrScale;
          }

          DirectX::XMFLOAT3 offset = DirectX::XMFLOAT3(0, 0, 0);
          float vertFlip = params.customProj == nullptr ? 1.0f : -1.0f;
          if (params.hAlign == Alignment::CENTER)
              offset.x -= parse.cursor.size.x / 2;
          else if (params.hAlign == Alignment::RIGHT)
              offset.x -= parse.cursor.size.x;
          if (params.vAlign == Alignment::CENTER)
              offset.y -= parse.cursor.size.y / 2 * vertFlip;
          else if (params.vAlign == Alignment::BOTTOM)
              offset.y -= parse.cursor.size.y * vertFlip;

          DirectX::XMMATRIX M = DirectX::XMMatrixTranslation(offset.x, offset.y, offset.z);
          M = M * DirectX::XMMatrixScaling(params.scale, params.scale, params.scale);
          M = M * DirectX::XMMatrixRotationZ(params.rot);

          if (params.customRot != nullptr)
              M = M * (*params.customRot);

          M = M * DirectX::XMMatrixTranslation(params.pos.x, params.pos.y, params.pos.z);

          if (params.customProj != nullptr) {
              M = DirectX::XMMatrixScaling(1, -1, 1) * M; // Invert Y for the custom matrix to make sense
              M = M * (*params.customProj);
          } else {
              assert(internal::canvas.width > 0);
              assert(internal::canvas.height > 0);
              assert(internal::canvas.GetDPI() > 0);
              M = M * internal::canvas.GetProjection();
          }

          if (params.shadowColor.w > 0) {
              DirectX::XMStoreFloat4x4(&font.transform,
                                       DirectX::XMMatrixTranslation(params.shadowOffsetX, params.shadowOffsetY, 0) * M);
              font.color = params.shadowColor;
              font.bolden = params.bolden;
              font.softness = params.softness * 0.5f;

              iface->BindDynamicConstants(font, BUFFER_FONT, cmd);
              iface->DrawInstanced(4, parse.quads, 0, 0, cmd);
          }

          DirectX::XMStoreFloat4x4(&font.transform, M);
          font.color = params.color;
          font.bolden = params.bolden;
          font.softness = params.softness;

          iface->BindDynamicConstants(font, BUFFER_FONT, cmd);
          iface->DrawInstanced(4, parse.quads, 0, 0, cmd);

          iface->EventEnd(cmd);
      }

      return parse.cursor;
  }

  void SetCanvas(const rx::Canvas &newCanvas) {
      internal::canvas = newCanvas;
  }

  Cursor Draw(const char *text, size_t len, const rx::Font::RenderMode &params, ThreadCommands cmd) {
      return DrawInternal(text, len, params, cmd);
  }
  Cursor Draw(const wchar_t *text, size_t len, const rx::Font::RenderMode &params, ThreadCommands cmd) {
      return DrawInternal(text, len, params, cmd);
  }
  Cursor Draw(const char *text, const rx::Font::RenderMode &params, ThreadCommands cmd) {
      return DrawInternal(text, strlen(text), params, cmd);
  }
  Cursor Draw(const wchar_t *text, const rx::Font::RenderMode &params, ThreadCommands cmd) {
      return DrawInternal(text, wcslen(text), params, cmd);
  }
  Cursor Draw(const std::string &text, const rx::Font::RenderMode &params, ThreadCommands cmd) {
      return DrawInternal(text.c_str(), text.length(), params, cmd);
  }
  Cursor Draw(const std::wstring &text, const rx::Font::RenderMode &params, ThreadCommands cmd) {
      return DrawInternal(text.c_str(), text.length(), params, cmd);
  }

  DirectX::XMFLOAT2 TextSize(const char *text, size_t len, const rx::Font::RenderMode &params) {
      if (len == 0)
          return {0, 0};
      return internal::ParseText(text, len, params).cursor.size;
  }

  DirectX::XMFLOAT2 TextSize(const wchar_t *text, size_t len, const rx::Font::RenderMode &params) {
      if (len == 0)
          return {0, 0};
      return internal::ParseText(text, len, params).cursor.size;
  }

  DirectX::XMFLOAT2 TextSize(const char *text, const rx::Font::RenderMode &params) {
      size_t len = strlen(text);
      if (len == 0)
          return {0, 0};
      return internal::ParseText(text, len, params).cursor.size;
  }

  DirectX::XMFLOAT2 TextSize(const wchar_t *text, const rx::Font::RenderMode &params) {
      size_t len = wcslen(text);
      if (len == 0)
          return {0, 0};
      return internal::ParseText(text, len, params).cursor.size;
  }

  DirectX::XMFLOAT2 TextSize(const std::string &text, const rx::Font::RenderMode &params) {
      if (text.length() == 0)
          return {0, 0};
      return internal::ParseText(text.c_str(), text.length(), params).cursor.size;
  }

  DirectX::XMFLOAT2 TextSize(const std::wstring &text, const rx::Font::RenderMode &params) {
      if (text.length() == 0)
          return {0, 0};
      return internal::ParseText(text.c_str(), text.length(), params).cursor.size;
  }

  float TextWidth(const char *text, size_t text_length, const rx::Font::RenderMode &params) {
      return TextSize(text, text_length, params).x;
  }
  float TextWidth(const wchar_t *text, size_t text_length, const rx::Font::RenderMode &params) {
      return TextSize(text, text_length, params).x;
  }
  float TextWidth(const char *text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).x;
  }
  float TextWidth(const wchar_t *text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).x;
  }
  float TextWidth(const std::string &text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).x;
  }
  float TextWidth(const std::wstring &text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).x;
  }

  float TextHeight(const char *text, size_t text_length, const rx::Font::RenderMode &params) {
      return TextSize(text, text_length, params).y;
  }
  float TextHeight(const wchar_t *text, size_t text_length, const rx::Font::RenderMode &params) {
      return TextSize(text, text_length, params).y;
  }
  float TextHeight(const char *text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).y;
  }
  float TextHeight(const wchar_t *text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).y;
  }
  float TextHeight(const std::string &text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).y;
  }
  float TextHeight(const std::wstring &text, const rx::Font::RenderMode &params) {
      return TextSize(text, params).y;
  }

  rx::Texture* FontManager::getAtlasTexture() {
      return &internal::texture;
  }

  using namespace ShadowEngine; // Resource, ResourceType, Path

  const ResourceType FontResource::TYPE("font");

  FontResource::FontResource(const Path& path, ResourceTypeManager& manager)
    : Resource(path, manager), fileData() {
  }

  bool FontResource::load(size_t size, const uint8_t *mem) {
      if (size == 0) return false;

      fileData.resize(size);
      memcpy(fileData.dataMut(), mem, size);
      return true;
  }

  FontManager::FontManager()
    : ResourceTypeManager() {}

  Resource* FontManager::createResource(const Path& path) {
      return new FontResource(path, *this);
  }

  void FontManager::destroyResource(Resource& res) {
      free(static_cast<FontResource*>(&res));
  }

}