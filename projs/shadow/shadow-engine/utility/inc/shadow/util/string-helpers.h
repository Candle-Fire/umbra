#pragma once

#include <string>
#include <vector>

#include "shadow/exports.h"

namespace SH {
  // String manipluation utilities.
  // Because std::string is heavily lacking.
  namespace Util::Str {
    // Convert the string to lower case, return a new string.
    // This only works in ASCII encoding.
    std::string API toLower(const std::string &str);

    std::vector<std::string> API explode(const std::string &s, const char &c);

    std::string API substr_range(std::string const &str, size_t start, size_t end);
  }
}