#pragma once

#include <string>
#include <fs/hash.h>

namespace ShadowEngine {
    /**
     * Stores split data about a path, for easy referencing and decomposition.
     * Not to be used as a replacement for the Path class.
     */
    struct PathInfo {
        explicit PathInfo(std::string& str);

        char extension[10];
        char baseName[256];
        char directory[256];
        char domain[256];
        char prelude[10];
    };

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
        static std::string normalise(std::string& path);
        // Get the prelude of the given path.
        static std::string getPrelude(std::string& path);
        // Get the domain of the given path.
        static std::string getDomain(std::string& path);
        // Get the directory of the given path.
        static std::string getDirectory(std::string& path);
        // Get the name of the file of the given path.
        static std::string getFilename(std::string& path);
        // Get the file extension of the given path.
        static std::string getExtension(std::string& path);
        // Check if the path has the given extension.
        static bool hasExtension(std::string& path, std::string& ext);
        // Replace the extension of the given path.
        static std::string replaceExtension(std::string& path, std::string& newExt);

        Path();
        explicit Path(const std::string& str);

        void operator=(const std::string& rhs);
        bool operator==(const std::string& rhs);
        bool operator==(const Path& rhs);
        bool operator!=(const Path& rhs);

        // Use this to set a new value into the path; it handles the hash too.
        void set(const std::string& path);

        uint32_t length() const { return path.length(); };
        PathHash getHash() { return hash; }
        const char* c_str() const { return path.data(); }
        bool isEmpty() const { return path.length() == 0; }
    private:
        std::string path;
        PathHash hash;
    };
}