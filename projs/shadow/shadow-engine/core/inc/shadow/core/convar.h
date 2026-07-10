#pragma once
#include <cstdint>
#include <string>

class ConVar;
extern ConVar *firstVarInAssembly;

class ConVar
{
  friend class ConVarManager;

  std::uint64_t data = 0;

  ConVar* next;

  std::string name;
  std::string description;
public:
  ConVar(const std::string& name, std::uint64_t default_value, const std::string& description) noexcept;

  void SetValue(char* value);
  std::uint64_t GetValue();
};




class ConVarManager
{
  ConVar *first;

  ConVarManager();

public:
  static ConVarManager* Get();

  ConVar* GetByName(std::string name);

  void ParseArgs(int argc, char *argv[]);
};
