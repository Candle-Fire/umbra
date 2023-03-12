#include <fs/path.h>
#include <string.h>
#include <str/string.h>

namespace ShadowEngine {

    Path::Path() : path {} { }

    Path::Path(const std::string &str) {
        set(normalise((std::string&) str));
    }

    void Path::set(const std::string &str) {
#ifdef _WIN32
        std::string temp = Str::toLower((std::string&) str);
        hash = PathHash(temp);
#else
        hash = PathHash(str);
#endif
        path = str;
    }

    Path& Path::operator=(const std::string &rhs) {
        set(rhs);
        return *this;
    }

    bool Path::operator==(const std::string &rhs) {
        return path == rhs;
    }

    bool Path::operator==(const ShadowEngine::Path &rhs) {
        return path == rhs.path;
    }

    bool Path::operator!=(const ShadowEngine::Path &rhs) {
        return path != rhs.path;
    }

    std::string Path::normalise(std::string &str) {
        bool prevSlash = false;

        std::string temp;
        const char* path = str.c_str();
        size_t len = str.length();
        size_t i = 0;

        // Skip initial stuff.
        size_t ind = str.find_first_of(":");
        path += ind;
        if (path[0] == '.' && (path[1] == '\\' || path[1] == '/'))
            path += 2;
#ifdef _WIN32
        if (path[0] == '\\' || path[0] == '/')
            ++path;
#endif

        while (*path != '\0' && i < len) {
            bool slash = *path == '\\' || *path == '/';

            // Skip double slashes.
            if (slash && prevSlash) {
                path++; continue;
            }

            // Convert backslashes to forward slashes.
            temp.append(std::to_string(*path == '\\' ? '/' : *path));

            path++; i++; prevSlash = slash;
        }

        return temp;
    }

    std::string Path::getPrelude(std::string &path) {
        return path.substr(0, path.find_first_of(":"));
    }

    std::string Path::getDomain(std::string &path) {
        return path.substr(path.find_first_of(":"), path.find_first_of("/"));
    }

    std::string Path::getDirectory(std::string &path) {
        return path.substr(path.find_first_of(":"), path.find_last_of("/"));
    }

    std::string Path::getFilename(std::string &path) {
        return path.substr(path.find_last_of("/"), path.find_last_of("."));
    }

    std::string Path::getExtension(std::string &path) {
        return path.substr(path.find_last_of("."), path.length());
    }

    std::string Path::replaceExtension(std::string &path, std::string &newExt) {
        return path.substr(0, path.length() - newExt.length()).append(newExt);
    }

    bool Path::hasExtension(std::string &path, std::string &ext) {
        return path.find_last_of(ext) == path.length() - ext.length();
    }

    PathInfo::PathInfo(std::string &str) {
        std::string normalised = Path::normalise(str);

        std::string preludeS = Path::getPrelude(normalised);
        memcpy_s(prelude, 10, preludeS.c_str(), preludeS.length());
        std::string domainS = Path::getDomain(normalised);
        memcpy_s(domain, 256, domainS.c_str(), domainS.length());
        std::string directoryS = Path::getDirectory(normalised);
        memcpy_s(directory, 256, directoryS.c_str(), directoryS.length());
        std::string filenameS = Path::getFilename(normalised);
        memcpy_s(baseName, 256, filenameS.c_str(), filenameS.length());
        std::string extensionS = Path::getExtension(normalised);
        memcpy_s(extension, 10, extensionS.c_str(), extensionS.length());
    }

}