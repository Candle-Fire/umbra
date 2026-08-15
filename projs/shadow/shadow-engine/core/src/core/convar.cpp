#include "shadow/core/convar.h"

ConVar *firstVarInAssembly;

ConVar::ConVar(const std::string& name, std::uint64_t default_value, const std::string& description) noexcept:
  data(default_value), name(name), description(description)
{
  this->next = firstVarInAssembly;
  firstVarInAssembly = this;
}

void ConVar::SetValue(const char* value)
{
  this->data = std::atoi(value);
}

std::uint64_t ConVar::GetValue()
{
  return this->data;
}

ConVarManager::ConVarManager(): convars(firstVarInAssembly) {}

void ConVarManager::AddConVars(ConVar* first)
{
  this->convars.concat({first});
  this->UpdateConVars();
}

ConVarManager* ConVarManager::Get()
{
  static auto* manager = new ConVarManager();
  return manager;
}

ConVar* ConVarManager::GetByName(std::string name)
{
  for (ConVar* convar : convars)
  {
    if (convar->name == name)
      return convar;
  }

  return nullptr;
}

void ConVarManager::ParseArgs(int argc, char* argv[])
{
  if (argc <= 1)
  {
    return;
  }

  for (size_t i = 1; i < argc; i++) {
    std::string param(argv[i]);
    if (param.starts_with("-V"))
    {
      size_t const pos = param.find('=');
      auto name = param.substr(2, pos-2);

      std::string value;
      if (pos != std::string::npos)
        value = param.substr(pos + 1);
      else
        value = argv[i+1];

      this->args.emplace(name, value);
    }
  }
  this->UpdateConVars();
}

void ConVarManager::UpdateConVars()
{
  for (auto &arg : this->args)
  {
    const auto convar = this->GetByName(arg.first);
    if (convar != nullptr)
    {
      convar->SetValue(arg.second.c_str());
    }
  }
}

