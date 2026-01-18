#pragma once
#include <string>

namespace SH {
    // String manipluation utilities.
    // Because std::string is heavily lacking.
    namespace Str {
        // Convert the string to lower case, return a new string.
        // This only works in ASCII encoding.
        std::string toLower(const std::string& str);

        std::wstring toWide(const std::string& str);

        std::string toShort(const std::wstring& str);
    }
}