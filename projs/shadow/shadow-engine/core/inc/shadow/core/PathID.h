#pragma once

#include <string>
#include <cstdint>
#include <filesystem>

#include "shadow/util/hash.h"
#include "shadow/exports.h"

namespace SH {
  /**
   * Stores split data about a path, for easy referencing and decomposition.
   * Not to be used as a replacement for the Path class.
   */
  struct PathInfo {
    explicit PathInfo(std::string &str);

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
   * All static operations are copy-instantiated.
   * All instance operations are in-place assignments.
   * A typical absolute path is of the form:
   *  prelude:/domain/directory/filename.extension
   * A typical relative path is of the form:
   *  directory/filename.extension
   */
  struct API Path {
    // Make sure the path is valid.
    // Always from the root.
    // One slash separating.
    static std::string normalise(std::string &path);
    SH::Path normalise();

    // Get the prelude of the given path.
    static std::string getPrelude(std::string &path);
    SH::Path getPrelude();

    // Get the domain of the given path.
    static std::string getDomain(const std::string &path);
    SH::Path getDomain();

    // Get the directory of the given path.
    static std::string getDirectory(const std::string &path);
    SH::Path getDirectory();

    // Get the name of the file of the given path.
    static std::string getFilename(std::string &path);
    SH::Path getFilename();

    // Get the file extension of the given path.
    static std::string getExtension(std::string &path);
    SH::Path getExtension();

    // Check if the path has the given extension.
    static bool hasExtension(std::string &path, std::string &ext);
    bool hasExtension(std::string& ext);

    // Replace the extension of the given path.
    static std::string replaceExtension(std::string &path, const std::string &newExt);
    SH::Path& replaceExtension(const std::string& newExt);

    // Get file without ext
    static std::string removeExtension(std::string& path);
    SH::Path& removeExtension();

    // Get absolute path from drive root
    static std::string makeAbsolute(std::string& path);
    SH::Path& makeAbsolute();

    Path();

    Path(const std::string &str);

    Path &operator=(const std::string &rhs);

    bool operator==(const std::string &rhs) const;

    bool operator==(const Path &rhs) const;

    bool operator!=(const Path &rhs) const;

    bool operator<(const Path &rhs) const;

    Path operator+(const Path& rhs) const;
    Path& operator+=(const Path& rhs);

    operator std::string() const;
    operator std::string&();

    operator std::filesystem::path() const;

    // Use this to set a new value into the path; it handles the hash too.
    void set(const std::string &path);

    [[nodiscard]] uint32_t length() const { return path.length(); };

    [[nodiscard]] PathHash getHash() const { return hash; }

    [[nodiscard]] const char *c_str() const { return path.data(); }

    [[nodiscard]] std::string const &get() const { return path; }

    [[nodiscard]] bool isEmpty() const { return path.length() == 0; }

    std::string toString() const { return path; }

  private:
    std::string path;
    PathHash hash;
  };

  Path API operator ""_id(const char *path, size_t length);
}

using SH::operator ""_id;