#pragma once
#include <memory>
#include <core/Module.h>

namespace vlkx {

    class RenderOrchestrator : public ShadowEngine::Module {
      SHObject_Base(RenderOrchestrator)

      std::vector<std::shared_ptr<ShadowEngine::SubmitterModule>> submitters;
      std::weak_ptr<ShadowEngine::SubmitterModule> activeSubmitter;

      public:
        void Init() override;

        std::weak_ptr<ShadowEngine::SubmitterModule>& getSubmitter() { return activeSubmitter; }

    };
}