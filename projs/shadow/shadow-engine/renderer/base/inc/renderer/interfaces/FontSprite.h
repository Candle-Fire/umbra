#pragma once

#include <string>
#include "Font.h"

namespace rx {

  // Represents a Font String as a Sprite.
  class FontSprite {
  private:
    enum Flags {
      EMPTY = 0,
      HIDDEN = 1 << 0,
      STATIC = 1 << 1
    };
    uint32_t flags = Flags::EMPTY;

  public:
    std::wstring text;
    rx::Font::RenderMode params;
    Font::FontResource* fontResource;
    std::string fontName;

    FontSprite() = default;
    FontSprite(const std::string& text, const rx::Font::RenderMode& params = rx::Font::RenderMode(), const std::string& fontName = "")
        : params(params), fontName(fontName) {
        SetText(text);
        if (!fontName.empty())
            fontResource = ShadowEngine::ResourceManager::Get().load<rx::Font::FontResource>(ShadowEngine::Path(fontName));
    }
    virtual ~FontSprite();

    virtual void FixedUpdate();
    virtual void Update(float dt);
    virtual void Draw(rx::ThreadCommands cmd) const;

    constexpr void SetHidden(bool val = true) { if (val) flags |= Flags::HIDDEN; else flags &= ~Flags::HIDDEN; }
    constexpr bool IsHidden() const { return flags & Flags::HIDDEN; }
    constexpr void SetDisabled(bool val = true) { if (val) flags |= Flags::STATIC; else flags &= ~Flags::STATIC; }
    constexpr bool IsDisabled() const { return flags & Flags::STATIC; }

    DirectX::XMFLOAT2 TextSize() const;
    float TextWidth() const;
    float TextHeight() const;

    void SetText(const std::string& val);
    void SetText(std::string&& val);
    void SetText(const std::wstring& val);
    void SetText(std::wstring&& val);

    std::string GetTextA() const;
    const std::wstring& GetText() const;

  };

}