#include "UUID.h"
#include <assert.h>

#ifdef WIN32
#include <combaseapi.h>
#endif

#ifdef linux
#include <uuid/uuid.h>
#endif

/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

namespace sonic {

  bool UUID::IsValidUUID(const char *string) {
    size_t len = strlen(string);

    // UUID strings are 36 characters long
    if (len != 36)
      return false;

    for (size_t i = 0; i < len; i++) {
      const char c = string[i];
      // Check for dashes not in the correct positions
      if (i == 8 || i == 13 || i == 18 || i == 23) {
        if (c != '-')
          return false;
      }
      // Check for dashes in incorrect positions
      if (c == '-') {
        if (i != 8 && i != 13 && i != 18 && i != 23)
          return false;
      } else if (!isxdigit(c)) {
        return false;
      }
    }

    return true;
  }

  UUID::UUID(const char *str) {
    Clear();
    if (!IsValidUUID(str)) return;

    size_t len = strlen(str);

    // Bytes are 2 characters, so track whether we're on the first or second. True if we "have" the first character in the byte.
    bool hasChar = false;
    // The first character of a double byte.
    char c0 = '\0';
    // The number of bytes we've accumulated so far.
    size_t bytes = 0;

    for (size_t i = 0; i < len; i++) {
      const char c = str[i];
      if (c == '-')
        continue;

      if (!hasChar) {
        c0 = c;
        hasChar = true;
      } else {
        data.u8[bytes++] = byteFromHex(c0, c);
        hasChar = false;
      }
    }
  }

  UUID UUID::Generate() {
    UUID id;
#ifdef WIN32
    CoCreateGuid( (GUID*) &id);
#elif defined(linux)
    uuid_t uuid;
    uuid_generate_random(&uuid);

    memset(&id.data.u8, uuid.__u_bits, 16);
#endif
    return id;
  }
}
