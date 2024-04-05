#pragma once

#include <DirectXMath.h>
#include "renderer/GraphicsDefine.h"
#include "renderer/Canvas.h"
#include "renderer/Interface.h"
#include "renderer/assets/RenderResource.h"
#include "shadow/assets/fs/path.h"
#include "shadow/assets/resource/Resource.h"
#include <shadow/assets/resource/ResourceManager.h>
#include "imstb_truetype.h"
#include "spdlog/spdlog.h"
#include "shadow/core/ShadowApplication.h"
#include <shadow/exports.h>

namespace rx::Font {

  enum Alignment {
    LEFT,
    CENTER,
    RIGHT,
    TOP,
    BOTTOM
  };

  static constexpr int DefaultSize = 16;

  struct Cursor {
    DirectX::XMFLOAT2 pos = {};
    DirectX::XMFLOAT2 size = {};
  };

  struct RenderMode {
    DirectX::XMFLOAT3 pos = {};
    int size = DefaultSize;
    float scale = 1;
    float rot = 0;
    float spacingX = 0;
    float spacingY = 0;
    Alignment hAlign = Alignment::LEFT;
    Alignment vAlign = Alignment::TOP;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT4 shadowColor;
    float hWrap = -1;
    int style = 0;

    // SDF mode
    float softness = 0.1f;
    float bolden = 0;
    float shadowSoftness = 0.5f;
    float shadowBolden = 0.1f;
    float shadowOffsetX = 0;
    float shadowOffsetY = 0;
    Cursor cursor;
    float hdrScale = 1.0f;
    const DirectX::XMMATRIX *customProj = nullptr;
    const DirectX::XMMATRIX *customRot = nullptr;

    enum Flags {
      EMPTY = 0,
      SDF = 1 << 0,
      COLORSPACE_HDR10 = 1 << 1,
      COLORSPACE_LINEAR = 1 << 2,
      DEPTH_TEST = 1 << 3
    };

    Flags flags = Flags::SDF;

    // enable-bitmap-operators trait doesn't work because we're a nested struct, and there's nowhere for the global declaration to go

    constexpr bool isSDFEnabled() const { return flags & Flags::SDF; }
    constexpr bool isHDR10Enabled() const { return flags & Flags::COLORSPACE_HDR10; }
    constexpr bool isLinearEnabled() const { return flags & Flags::COLORSPACE_LINEAR; }
    constexpr bool isDepthTestEnabled() const { return flags & Flags::DEPTH_TEST; }

    constexpr void enableSDF() { flags = static_cast<Flags>(flags | Flags::SDF); }
    constexpr void enableHDR10() { flags = static_cast<Flags>(flags | Flags::COLORSPACE_HDR10); }
    constexpr void enableLinear(float scaling = 1.0f) {
        flags = static_cast<Flags>(flags | Flags::COLORSPACE_LINEAR);
        hdrScale = scale;
    }
    constexpr void enableDepthTest() { flags = static_cast<Flags>(flags | Flags::DEPTH_TEST); }

    constexpr void disableSDF() { flags = static_cast<Flags>(flags & Flags::SDF); }
    constexpr void disableHDR10() { flags = static_cast<Flags>(flags & Flags::COLORSPACE_HDR10); }
    constexpr void disableLinear() { flags = static_cast<Flags>(flags & Flags::COLORSPACE_LINEAR); }
    constexpr void disableDepthTest() { flags = static_cast<Flags>(flags & Flags::DEPTH_TEST); }

    RenderMode(float x = 0,
               float y = 0,
               int size = DefaultSize,
               Alignment hAlignment = Alignment::LEFT,
               Alignment vAlignment = Alignment::TOP,
               DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(255, 255, 255, 255),
               DirectX::XMFLOAT4 shadowColor = DirectX::XMFLOAT4(0, 0, 0, 0)) :
        pos(x, y, 0), size(size), hAlign(hAlignment), vAlign(vAlignment), color(color), shadowColor(shadowColor) {}

    RenderMode(DirectX::XMFLOAT4 color,
               DirectX::XMFLOAT4 shadowColor = DirectX::XMFLOAT4(0, 0, 0, 0),
               float softness = 0.08,
               float bolden = 0,
               float shadowSoftness = 0.5f,
               float shadowBolden = 0.1f,
               float shadowOffsetX = 0,
               float shadowOffsetY = 0) :
        color(color),
        shadowColor(shadowColor),
        softness(softness),
        bolden(bolden),
        shadowSoftness(shadowSoftness),
        shadowBolden(shadowBolden),
        shadowOffsetX(shadowOffsetX),
        shadowOffsetY(shadowOffsetY) {}
  };

  // Prepare all font usage
  void Initialize();

  // Get the texture storing the currently used fonts
  const rx::Texture *GetAtlas();

  // Add a font from a file. Return the font handle ID
  int AddFont(const std::string &file);

  // Add a font from memory. Return the font handle ID
  int AddFont(const std::string &name, const uint8_t *data, size_t size);

  // Update command list
  void SetRenderPlate(const rx::Canvas &newCanvas);

  // Internal: update the atlas for new textures
  void UpdateAtlas(float upscaling = 1.0f);

  Cursor Draw(const char *text, size_t length, const RenderMode &render, rx::ThreadCommands commands);
  Cursor Draw(const wchar_t *text, size_t length, const RenderMode &render, rx::ThreadCommands commands);
  Cursor Draw(const char *text, const RenderMode &render, rx::ThreadCommands commands);
  Cursor Draw(const wchar_t *text, const RenderMode &render, rx::ThreadCommands commands);
  Cursor Draw(const std::string &text, const RenderMode &render, rx::ThreadCommands commands);
  Cursor Draw(const std::wstring &text, const RenderMode &render, rx::ThreadCommands commands);

  DirectX::XMFLOAT2 TextSize(const char *text, size_t length, const RenderMode &render);
  DirectX::XMFLOAT2 TextSize(const wchar_t *text, size_t length, const RenderMode &render);
  DirectX::XMFLOAT2 TextSize(const char *text, const RenderMode &render);
  DirectX::XMFLOAT2 TextSize(const wchar_t *text, const RenderMode &render);
  DirectX::XMFLOAT2 TextSize(const std::string &text, const RenderMode &render);
  DirectX::XMFLOAT2 TextSize(const std::wstring &text, const RenderMode &render);

  float TextWidth(const char *text, size_t length, const RenderMode &render);
  float TextWidth(const wchar_t *text, size_t length, const RenderMode &render);
  float TextWidth(const char *text, const RenderMode &render);
  float TextWidth(const wchar_t *text, const RenderMode &render);
  float TextWidth(const std::string &text, const RenderMode &render);
  float TextWidth(const std::wstring &text, const RenderMode &render);

  float TextHeight(const char *text, size_t length, const RenderMode &render);
  float TextHeight(const wchar_t *text, size_t length, const RenderMode &render);
  float TextHeight(const char *text, const RenderMode &render);
  float TextHeight(const wchar_t *text, const RenderMode &render);
  float TextHeight(const std::string &text, const RenderMode &render);
  float TextHeight(const std::wstring &text, const RenderMode &render);

  using namespace ShadowEngine;

  struct FontStyle {
    std::string name;
    std::vector<uint8_t> fontBuffer;
    stbtt_fontinfo fontInfo;
    // asc: ascent, dsc: descent, gap: lineGap
    int asc, dsc, gap;
    void Create(const std::string &newName, const uint8_t *data, size_t size) {
        name = newName;
        int offset = stbtt_GetFontOffsetForIndex(data, 0);
        if (!stbtt_InitFont(&fontInfo, data, offset))
            spdlog::error("Failed to load font " + name + " because it was of an unexpected format - expected ttf.");

        stbtt_GetFontVMetrics(&fontInfo, &asc, &dsc, &gap);
    }

    void Create(const std::string &fileName) {
        OutputMemoryStream oms;
        SH::ShadowApplication::diskFS->readSync(ShadowEngine::Path(fileName), oms);

        if (oms.empty())
            spdlog::error("Failed to load font " + fileName + " because it could not be opened.");
        else
            Create(fileName, oms.data(), oms.size());
    }
  };

  struct API FontResource final : Resource {
    FontResource(const Path &path, ResourceTypeManager &manager);
    ShadowEngine::ResourceType getType() const override { return TYPE; }

    void unload() override { fileData.free(); }
    bool load(size_t size, const uint8_t *mem) override;

    OutputMemoryStream fileData;
    static const ResourceType TYPE;
  };

  struct API FontManager final : ResourceTypeManager {
    friend struct FontResource;
  public:
    FontManager();
    ~FontManager();

    rx::Texture *getAtlasTexture();

  private:
    virtual ShadowEngine::Resource* createResource(const ShadowEngine::Path &path) override;
    void destroyResource(ShadowEngine::Resource &res) override;

  };
}