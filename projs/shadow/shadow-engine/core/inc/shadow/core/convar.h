#pragma once
#include <cstdint>
#include <map>
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

  void SetValue(const char* value);
  std::uint64_t GetValue();
};




/**
 * @brief Class responsible for
 */
class ConVarManager
{

  /**
   * @brief The list of console vars known to the system
   */
  linked_list<ConVar> convars;

  /**
   * @brief The known convar settings the app was started with.
   * Use the @see { ParseArgs } to parse the argv
   */
  std::map<std::string, std::string> args;

  ConVarManager();

public:

  /**
   * @brief Adds a list of convars to the manager
   * @param first The pointer to the first item in the linked list
   */
  void AddConVars(ConVar* first);


  /**
   * @brief Returns the singleton instance of the manager
   * @return The pointer to the singleton manger
   */
  static ConVarManager* Get();

  ConVar* GetByName(std::string name);

  void ParseArgs(int argc, char *argv[]);

private:
  void UpdateConVars();
};
