#include <renderer/RenderOrchestrator.h>
#include "core/ShadowApplication.h"


template<typename Base, typename T>
inline bool is_instance_of(const T *ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

namespace vlkx {

    MODULE_ENTRY(RenderOrchestrator, RenderOrchestrator)

    SHObject_Base_Impl(RenderOrchestrator)

    void RenderOrchestrator::Init() {
        auto modules = ShadowEngine::ShadowApplication::Get().GetModuleManager();

        for (auto& module : modules.GetModules()) {
            if (is_instance_of<ShadowEngine::SubmitterModule>(module.module.get())) {
                auto* mod = dynamic_cast<ShadowEngine::SubmitterModule *>(module.module.get());
                submitters.emplace_back(mod);
            }
        }

        // Prefer the editor if it's available
        for (const auto& module : submitters) {
            if (module->GetType().ends_with("editor")) activeSubmitter = module;
        }

        // Fall back to whatever was added first.
        if (activeSubmitter.expired()) activeSubmitter = submitters[0];
    }
}