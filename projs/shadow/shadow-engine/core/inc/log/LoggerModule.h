#pragma once
#include "core/Module.h"


// Core log macros
#define SH_CORE_TRACE(...)    ::ShadowEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SH_CORE_INFO(...)     ::ShadowEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SH_CORE_WARN(...)     ::ShadowEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SH_CORE_ERROR(...)    ::ShadowEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SH_CORE_CRITICAL(...) ::ShadowEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SH_TRACE(...)         ::ShadowEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SH_INFO(...)          ::ShadowEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define SH_WARN(...)          ::ShadowEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SH_ERROR(...)         ::ShadowEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define SH_CRITICAL(...)      ::ShadowEngine::Log::GetClientLogger()->critical(__VA_ARGS__)

namespace SH {

    class LoggerModule;

    LoggerModule *Logger;

    class LoggerModule : SH::Module {

      SHObject_Base(LoggerModule)

      public:
        LoggerModule() {
            SH::Logger = this;
        }

        void Init() override {
            Module::Init();

        }

    };

}

