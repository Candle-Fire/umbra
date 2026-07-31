#pragma once
#include <cstdint>
#include <string>

#include "shadow/util/linked_list.h"

class ConVar;
extern ConVar *firstVarInAssembly;

class ConVar
{
  friend class ConVarManager;

  std::uint64_t data = 0;

  std::string name;
  std::string description;
public:
  ConVar* next;

  ConVar(const std::string& name, std::uint64_t default_value, const std::string& description) noexcept;

  void SetValue(char* value);
  std::uint64_t GetValue();
};




class ConVarManager
{
  linked_list<ConVar> convars;

  ConVarManager();

public:
  void AddConVars(ConVar* first);

  static ConVarManager* Get();

  ConVar* GetByName(std::string name);

  void ParseArgs(int argc, char *argv[]);
};
