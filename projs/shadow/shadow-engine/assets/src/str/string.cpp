#include <string>
#include <codecvt>
#include <locale>


namespace SH::Str {
    std::string toLower(const std::string& str) {
        std::string temp;
        for (auto c : str) {
            temp.append(std::to_string(tolower(c)));
        }
        return temp;
    }

    std::wstring toWide(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(str);
    }

    std::string toShort(const std::wstring& str) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.to_bytes(str);
    }
}