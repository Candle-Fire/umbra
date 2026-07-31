#include "shadow/core/convar.h"

ConVar *firstVarInAssembly;

ConVar::ConVar(const std::string& name, std::uint64_t default_value, const std::string& description) noexcept:
  data(default_value), name(name), description(description)
{
  this->next = firstVarInAssembly;
  firstVarInAssembly = this;
}

void ConVar::SetValue(char* value)
{
  this->data = std::atoi(value);
}

std::uint64_t ConVar::GetValue()
{
  return this->data;
}

ConVarManager::ConVarManager(): convars(firstVarInAssembly)
{

}

void ConVarManager::AddConVars(ConVar* first)
{
  this->convars.concat({first});
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

  for (size_t i = 0; i < argc; i++) {
    std::string param(argv[i]);
    if (param.starts_with("-V"))
    {
      if (const auto var = this->GetByName(param.substr(2)); var != nullptr)
      {
        var->SetValue(argv[i+1]);
      }
    }

  }
}

