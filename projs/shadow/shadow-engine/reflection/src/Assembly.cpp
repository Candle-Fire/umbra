#include "runtime/Assembly.h"
#include "spdlog/spdlog.h"

namespace Runtime {

  Assembly::~Assembly() {
      if (this->lib)
          Native::close(this->lib);
  }

  bool Assembly::HasSymbol(const std::string &name) const {
      if (!this->lib || name.empty())
          return false;
      return Native::locate_symbol(this->lib, name.c_str()) != nullptr;
  }

  const SH::Path &Assembly::GetID() const {
      return this->id;
  }

  bool Assembly::IsLoaded() const {
      return this->lib != nullptr;
  }

  void Assembly::Load() {
      auto path = this->path + Native::filename_components::suffix;

      this->lib = Native::open(path.c_str());

      if (!this->lib) {
          spdlog::error("Could not load library \"{}\"\n{}", this->path, Native::get_error_description());
      }
  }

  void Assembly::RunEntry() {
      auto entry_name = this->entryDefaultSymbol;
      if (this->HasSymbol(Assembly::entryNameSymbol)) {
          entry_name = this->get_variable<const char *>(this->entryNameSymbol);
      }

      if (!this->HasSymbol(entry_name)) {
          spdlog::debug(R"(Could not find entry point for assembly "{}" tried "{}")", this->id.get(), entry_name);
          return;
      }

      auto entry = this->GetFunction<void()>(entry_name);
      entry();
  }
}