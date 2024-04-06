
#include <stringapiset.h>
#include "shadow/util/string-helpers.h"

namespace SH::Util::Str {

  #define CP_UTF8 65001

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

    std::string substr_range(std::string const &str, size_t start, size_t end) {
        return str.substr(start, end - start);
    }


    void StringConvert(const std::string& from, std::wstring& to) {
#ifdef _WIN32
        int num = MultiByteToWideChar(CP_UTF8, 0, from.c_str(), -1, NULL, 0);
        if (num > 0) {
            to.resize(size_t(num) - 1);
            MultiByteToWideChar(CP_UTF8, 0, from.c_str(), -1, &to[0], num);
        }
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
		    to = cv.from_bytes(from);
#endif // _WIN32
    }

    void StringConvert(const std::wstring& from, std::string& to) {
#ifdef _WIN32
        int num = WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, NULL, 0, NULL, NULL);
        if (num > 0) {
            to.resize(size_t(num) - 1);
            WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, &to[0], num, NULL, NULL);
        }
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
		    to = cv.to_bytes(from);
#endif // _WIN32
    }

    int StringConvert(const char* from, wchar_t* to, int dest_size_in_characters) {
#ifdef _WIN32
        int num = MultiByteToWideChar(CP_UTF8, 0, from, -1, NULL, 0);
        if (num > 0) {
            if (dest_size_in_characters >= 0) {
                num = std::min(num, dest_size_in_characters);
            }
            MultiByteToWideChar(CP_UTF8, 0, from, -1, &to[0], num);
        }
        return num;
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
		    auto result = cv.from_bytes(from).c_str();
		    int num = (int)cv.converted();
		    if (dest_size_in_characters >= 0) {
			      num = std::min(num, dest_size_in_characters);
		    }
		    std::memcpy(to, result, num * sizeof(wchar_t));
		    return num;
#endif // _WIN32
    }

    int StringConvert(const wchar_t* from, char* to, int dest_size_in_characters) {
#ifdef _WIN32
        int num = WideCharToMultiByte(CP_UTF8, 0, from, -1, NULL, 0, NULL, NULL);
        if (num > 0) {
            if (dest_size_in_characters >= 0) {
                num = std::min(num, dest_size_in_characters);
            }
            WideCharToMultiByte(CP_UTF8, 0, from, -1, &to[0], num, NULL, NULL);
        }
        return num;
#else
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
		    auto result = cv.to_bytes(from).c_str();
		    int num = (size_t)cv.converted();
		    if (dest_size_in_characters >= 0) {
			      num = std::min(num, dest_size_in_characters);
		    }
		    std::memcpy(to, result, num * sizeof(char));
		    return num;
#endif // _WIN32
    }
}