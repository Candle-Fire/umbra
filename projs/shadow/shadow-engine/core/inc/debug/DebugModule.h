//
// Created by dpete on 31/08/2022.
//

#ifndef UMBRA_DEBUGMODULE_H
#define UMBRA_DEBUGMODULE_H

#include <SDL_events.h>
#include "core/Module.h"

namespace ShadowEngine::Debug {

    class DebugModule : public Module {

        SHObject_Base(DebugModule)

        bool active;

    public:
        void Render() override;

        void PreInit() override {  };

        void Init() override  {  };

        void Update() override {  };

        void LateRender() override {  };

        void AfterFrameEnd() override {  };

        void PreRender() override { };

        void Destroy() override {};

        void Event(SDL_Event* e) override {};
    };

}

#endif //UMBRA_DEBUGMODULE_H
