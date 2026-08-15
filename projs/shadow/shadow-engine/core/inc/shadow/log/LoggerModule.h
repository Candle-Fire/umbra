#pragma once
#include <memory>

#include "shadow/core/convar.h"
#include "shadow/core/Module.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

namespace SH {

  extern ConVar var_logLevel;

  void InitBasicLogger();

  class LoggerModule : SH::Module {

    SHObject_Base(LoggerModule)

    std::shared_ptr<spdlog::sinks::sink> console_sink;

    public:
      LoggerModule();

      void Init() override;

      static std::shared_ptr<LoggerModule> Get();
  };


  class Logger
  {
    std::string source;
    std::shared_ptr<spdlog::logger> logger;

  public:
    Logger(std::string name);

    template<typename... Args>
    void log(spdlog::level::level_enum level, spdlog::format_string_t<Args...> fmt, Args &&...args) const
    {
      this->logger->log(level, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void trace(spdlog::format_string_t<Args...> fmt, Args &&...args) const
    {
      this->log(spdlog::level::trace, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void debug(spdlog::format_string_t<Args...> fmt, Args &&...args) const
    {
      this->log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void info(spdlog::format_string_t<Args...> fmt, Args &&...args) const
    {
      this->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void warn(spdlog::format_string_t<Args...> fmt, Args &&...args) const
    {
      this->log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }
    template<typename... Args>
    void error(spdlog::format_string_t<Args...> fmt, Args &&...args) const
    {
      this->log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

  };
}

