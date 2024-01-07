#pragma once

namespace rx {
    class Resource {
    public:
        std::shared_ptr<void> state;

        inline bool IsValid() const { return state.get() != nullptr; }

        const Texture& GetTexture() const;
        void SetTexture(const Texture& tex, int srgbSubresource = -1);

        void SetOutdated();
    };
}