//
// Created by dpete on 30/08/2022.
//

#ifndef UMBRA_SDL2MODULE_H
#define UMBRA_SDL2MODULE_H

#include "Module.h"
#include "ShadowWindow.h"

#include <SDL.h>

namespace ShadowEngine {

    class SDL2Module : public Module {
        SHObject_Base(SDL2Module)

    public:
        ShadowEngine::ShadowWindow* _window;

    private:
        void Init() override;

        void PreInit() override;

        void Update() override;

        void Render() override;

        void LateRender() override;

        std::string GetName() override;
    };

}

#endif //UMBRA_SDL2MODULE_H
