#pragma once

#include <string>
#include <cstdint>

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
   * All operations are copy-instantiated, nothing works in-place.
   * A typical path is of the form:
   *  prelude:/domain/directory/filename.extension
   */
  struct API Path {
    // Make sure the path is valid.
    // Always from the root.
    // One slash separating.
    static std::string normalise(std::string &path);

    // Get the prelude of the given path.
    static std::string getPrelude(std::string &path);

    // Get the domain of the given path.
    static std::string getDomain(const std::string &path);

    // Get the directory of the given path.
    static std::string getDirectory(const std::string &path);

    // Get the name of the file of the given path.
    static std::string getFilename(std::string &path);

    // Get the file extension of the given path.
    static std::string getExtension(std::string &path);

    // Check if the path has the given extension.
    static bool hasExtension(std::string &path, std::string &ext);

    // Replace the extension of the given path.
    static std::string replaceExtension(std::string &path, std::string &newExt);

    Path();

    explicit Path(const std::string &str);

    Path &operator=(const std::string &rhs);

    bool operator==(const std::string &rhs) const;

    bool operator==(const Path &rhs) const;

    bool operator!=(const Path &rhs) const;

    bool operator<(const Path &rhs) const;

    operator std::string() const;

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