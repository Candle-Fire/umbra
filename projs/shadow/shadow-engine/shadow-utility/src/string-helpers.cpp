
#include "string-helpers.h"

namespace SH::Util::Str {

    std::vector<std::string> explode(const std::string &s, const char &c) {
        std::string buff;
        std::vector<std::string> v;

        for (auto n : s) {
            if (n != c) buff += n;
            else if (n == c && buff != "") {
                v.push_back(buff);
                buff = "";
            }
        }
        if (buff != "") v.push_back(buff);

        return v;
    }

    std::string toLower(const std::string &str) {
        std::string temp;
        for (auto c : str) {
            temp.append(std::to_string(tolower(c)));
        }
        return temp;
    }
}