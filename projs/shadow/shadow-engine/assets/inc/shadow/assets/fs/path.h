#pragma once

#include <string>
#include "hash.h"
#include "shadow/exports.h"
#include "shadow/util/string-helpers.h"

namespace SH {
    /**
     * Stores split data about a path, for easy referencing and decomposition.
     * Not to be used as a replacement for the Path class.
     */
    struct PathInfo {
        explicit PathInfo(const std::string& str);

        char extension[10];
        char baseName[256];
        char directory[256];
        char domain[256];
        char prelude[10];
    };

    // File Paths are hashed using the 64-bit StableHash system.
    using PathHash = StableHash;

    /**
     * Stores and handles paths in the VFS.
     * All operations are copy-instantiated, nothing works in-place.
     * A typical path is of the form:
     *  prelude:/domain/directory/filename.extension
     */
    struct Path {
        // Make sure the path is valid.
        // Always from the root.
        // One slash separating.
        static std::string normalise(const std::string& path);
        // Get the prelude of the given path.
        static std::string getPrelude(const std::string& path);
        // Get the domain of the given path.
        static std::string getDomain(const std::string& path);
        // Get the directory of the given path.
        static std::string getDirectory(const std::string& path);
        // Get the name of the file of the given path.
        static std::string getFilename(const std::string& path);
        // Get the file extension of the given path.
        static std::string getExtension(const std::string& path);
        // Check if the path has the given extension.
        static bool hasExtension(const std::string& path, const std::string& ext);
        // Replace the extension of the given path.
        static std::string replaceExtension(const std::string& path, const std::string& newExt);

        Path();
        explicit Path(const std::string& str);
        template<typename... Args> explicit Path(Args... args);

        Path& operator=(const std::string& rhs);
        bool operator==(const std::string& rhs);
        bool operator==(const Path& rhs);
        bool operator!=(const Path& rhs);
        bool operator<(const Path &rhs) const;
        Path operator/(const std::string& rhs) { return Path(toString().append(rhs)); }
        Path operator/(const Path& rhs) { return Path(toString().append(rhs.toString())); }

        void createHash() {
#ifdef _WIN32
            hash = PathHash(Util::Str::toLower(path));
#else
            hash = PathHash(path);
#endif
        }

        // Use this to set a new value into the path; it handles the hash too.
        void set(const std::string& path) { this->path = path; createHash(); }
        Path append(std::string_view str) { return Path(toString().append(str));}

        template <typename... Args> void append(Args... args);

        uint32_t length() const { return path.length(); };
        PathHash getHash() const { return hash; }
        const char* c_str() const { return path.data(); }
        std::string const& get() const { return path; }
        std::string const& get() { return path; }
        bool isEmpty() const { return path.length() == 0; }
        std::string toString() const { return path; }
    private:
        std::string path;
        PathHash hash;
    };

    template <typename... Args> Path::Path(Args... args) {
        (path.append(args),...);
        createHash();
    }

    template <typename... Args> void Path::append(Args... args) {
        (path.append(args),...);
        createHash();
    }

    Path API operator ""_id(const char *path, size_t length);
}

inline SH::Path operator/(std::string& l, SH::Path& r) {
    return SH::Path(l) / r;
}

inline SH::Path operator/(const char* l, SH::Path& r) {
    return SH::Path(l) / r;
}

using SH::operator ""_id;