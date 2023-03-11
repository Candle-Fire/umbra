#include <id/UUID.h>

namespace SE {
    static_assert(sizeof(UUID) == 16, "UUID has incorrect size");

    /**
     * Verify that a string has the correct format;
     * XXXXXXXX-XXXX-XXX-XXXXX-XXXXXXXXXXXX
     *
     * The length must be 36.
     * There must be dashes at index 8, 13, 18 and 23.
     * @param str the input string
     * @return whether the UUID string is correctly formed
     */
    bool UUID::IsValidStr(const char *str) {
        size_t const len = strlen(str);
        if (len != 36) return false;

        for (size_t i = 0; i < len; i++) {
            char c = str[i];
            if (c == '-') {
                if (i != 8 && i != 13 && i != 18 && i != 23) return false;
            } else if (! std::isxdigit(c)) {
                return false;
            }
        }

        return true;
    }

    UUID::UUID(char const* str ) {
        // A single byte is two hex characters.
        // Store them here so that we can use them later.
        char c0 = '\0', c1;

        size_t const len = strlen( str );
        uint32_t byteIdx = 0;

        for (size_t i = 0; i < len; i++ ) {
            char const c = str[i];
            if ( c == '-' )
                continue;

            // Scan for pairs of characters.
            // Only assign a byte if two have been parsed.
            if (c0 == '\0') {
                c0 = c;
            } else {
                c1 = c;
                data.u8[byteIdx++] = std::stoi(std::string(c0, c1));
                // Reset the first char so that we can return to scanning a pair.
                c0 = '\0';
            }
        }
    }
}