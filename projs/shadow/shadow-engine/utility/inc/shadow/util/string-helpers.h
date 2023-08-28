#pragma once

#include <string>
#include <vector>

namespace SH {
    // String manipluation utilities.
    // Because std::string is heavily lacking.
    namespace Util::Str {
        // Convert the string to lower case, return a new string.
        // This only works in ASCII encoding.
        std::string toLower(const std::string &str);

        std::vector<std::string> explode(const std::string &s, const char &c);
    }
}