#pragma once

#include <SDL_video.h>
#include <glm/glm.hpp>
#include "glm/ext/matrix_clip_space.hpp"

namespace rx {

    /**
     * A DPI-aware rendering target.
     * Handles updating & rendering images.
     * Renderers should extend from this canvas.
     */
    struct Canvas {
        virtual ~Canvas() = default;

        uint32_t width = 0;
        uint32_t height = 0;
        float DPI = 96;
        float DPIScaling = 1;

        inline void init(uint32_t width, uint32_t height, float dpi = 96) {
            this->width = width;
            this->height = height;
            this->DPI = dpi;
        }
        inline void init(const Canvas& o) { *this = o; }
        inline void init(SDL_Window* w) {
            int wi, he;
            float dpi, hdpi, vdpi;
            SDL_GetWindowSize(w, &wi, &he);
            SDL_GetDisplayDPI(0, &dpi, &hdpi, &vdpi);
            init(wi, he, dpi);
        }

        inline float GetDPI() const { return DPI * DPIScaling; }
        inline float GetDPIScaling() const { return DPIScaling; }

        inline uint32_t LogicalToPhysical(float logical) const { return uint32_t(logical * GetDPIScaling()); }
        inline float PhysicalToLogical(uint32_t physical) const { return float(physical) / GetDPIScaling(); }

        inline uint32_t GetPhysicalWidth() const { return width; }
        inline uint32_t GetPhysicalHeight() const { return height; }

        inline float GetLogicalWidth() const { return PhysicalToLogical(GetPhysicalWidth()); }
        inline float GetLogicalHeight() const { return PhysicalToLogical(GetPhysicalHeight()); }

        inline glm::mat4 GetProjection() const {
            // TODO: XMMatrixOrthographicOffCenterLH(0, (float)GetLogicalWidth(), (float)GetLogicalHeight(), 0, -1, 1); but dxmath doesn't work.
            // TODO: DX has an inverse Z direction and a transposed matrix relative to glm
            // TODO: this will make everything mega weird
            // TODO: seriously, fix this
            // TODO: i'm not kidding.
            DirectX::XMMatrixOrthographicOffCenter()
            return glm::ortho(0.0f, (float) GetLogicalWidth(), (float) GetLogicalHeight(), 0.0f, -1.0f, 1.0f);
        }
    };
}