#include <string>



namespace ShadowEngine::Str {
    std::string toLower(const std::string& str) {
        std::string temp;
        for (auto c : str) {
            temp.append(std::to_string(tolower(c)));
        }
        return temp;
    }
}