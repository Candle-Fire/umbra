//
// Created by dpete on 31/08/2022.
//

#ifndef UMBRA_DEBUGMODULE_H
#define UMBRA_DEBUGMODULE_H

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
    };

}

#endif //UMBRA_DEBUGMODULE_H
