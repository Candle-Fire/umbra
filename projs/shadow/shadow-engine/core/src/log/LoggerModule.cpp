#include "shadow/log/LoggerModule.h"
#include "shadow/core/module-manager-v2.h"
#include "shadow/core/ShadowApplication.h"
#include "spdlog/sinks/stdout_color_sinks.h"

SHObject_Base_Impl(SH::LoggerModule)

MODULE_ENTRY(SH::LoggerModule, LoggerModule)

namespace SH
{
  ConVar var_logLevel("log_level", 2,"0 = Verbose");

  void InitBasicLogger()
  {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto my_logger = std::make_shared<spdlog::logger>("Boot", console_sink);

    spdlog::set_default_logger(my_logger);

    switch (var_logLevel.GetValue())
    {
    case 0:
      spdlog::set_level(spdlog::level::trace);
    case 1:
    default:
      spdlog::set_level(spdlog::level::debug);
    }
  }

  LoggerModule::LoggerModule()
  {
    this->console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto my_logger = std::make_shared<spdlog::logger>("Main Logger", console_sink);

    spdlog::set_default_logger(my_logger);

    switch (var_logLevel.GetValue())
    {
    case 0:
      spdlog::set_level(spdlog::level::trace);
    case 1:
    default:
      spdlog::set_level(spdlog::level::debug);
    }
  }

  void LoggerModule::Init()
  {
    Module::Init();
  }

  std::shared_ptr<LoggerModule> LoggerModule::Get()
  {
  }

  Logger::Logger(std::string name): source(name)
  {
    this->logger = spdlog::default_logger()->clone(this->source);
  }
}
