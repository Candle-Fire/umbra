#include "shadow/core/PathID.h"
#include <string>

#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <algorithm>
#include <stdexcept>

#ifndef __STDC_LIB_EXT1__
#define memcpy_s(dest, destsz, src, count) memcpy(dest, src, count)
#endif

#include "shadow/util/string-helpers.h"

namespace SH {

  Path::Path() : path{} {}

  Path::Path(const std::string &str) {
      set(normalise((std::string &) str));
  }

  void Path::set(const std::string &str) {
#ifdef _WIN32
      std::string temp = Util::Str::toLower((std::string &) str);
      hash = PathHash(temp);
#else
      hash = PathHash(str);
#endif
      path = str;
  }

  Path &Path::operator=(const std::string &rhs) {
      set(rhs);
      return *this;
  }

  bool Path::operator==(const std::string &rhs) const {
      return path == rhs;
  }

  bool Path::operator==(const SH::Path &rhs) const {
      return path == rhs.path;
  }

  bool Path::operator!=(const SH::Path &rhs) const {
      return path != rhs.path;
  }

  std::string Path::normalise(std::string &id) {
      size_t atPos = id.find('@');
      size_t colonPos = id.rfind(':');

      // If no colon is present, the path is not valid
      if (colonPos == std::string::npos) {
          throw std::invalid_argument("Missing colon in path");
      }

      std::string prelude, optionalNamespace, path;

      if (atPos != std::string::npos && atPos < colonPos) {
          prelude = id.substr(0, atPos);
          optionalNamespace = id.substr(atPos + 1, colonPos - atPos - 1);
      } else {
          prelude = id.substr(0, colonPos);
      }

      path = id.substr(colonPos + 1);

      // Replace backslashes with forward slashes in the path
      std::replace(path.begin(), path.end(), '\\', '/');

      if (optionalNamespace.empty()) {
          return prelude + ":" + path;
      } else {
          return prelude + "@" + optionalNamespace + ":" + path;
      }
  }

  std::string Path::getPrelude(std::string &path) {
      return Util::Str::substr_range(path, 0, path.find_first_of('@'));
  }

  std::string Path::getDomain(const std::string &id) {
      size_t atPos = id.find('@');
      size_t colonPos = id.rfind(':');

      // If no colon is present, the path is not valid
      if (colonPos == std::string::npos) {
          throw std::invalid_argument("Missing colon in path");
      }

      // If '@' exists and is before the last ':', then extract namespace
      if (atPos != std::string::npos && atPos < colonPos) {
          return id.substr(atPos + 1, colonPos - atPos - 1);
      }

      // No namespace (domain) found
      return "";
  }

  std::string Path::getDirectory(const std::string &path) {
      return Util::Str::substr_range(path, path.find_first_of(':') + 1, path.find_last_of('/'));
  }

  std::string Path::getFilename(std::string &path) {
      return path.substr(path.find_last_of('/') + 1);
  }

  std::string Path::getExtension(std::string &path) {
      return Util::Str::substr_range(path, path.find_last_of('.') + 1, path.length());
  }

  std::string Path::replaceExtension(std::string &path, std::string &newExt) {
      return Util::Str::substr_range(path, 0, path.length() - newExt.length()).append(newExt);
  }

  bool Path::hasExtension(std::string &path, std::string &ext) {
      return path.rfind(ext) == (path.length() - ext.length());
  }
  bool Path::operator<(const Path &rhs) const {
      return path < rhs.path;
  }

  Path::operator std::string() const {
      return path;
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

  Path operator ""_id(const char *path, size_t length) {
      return Path(path);
  }
}