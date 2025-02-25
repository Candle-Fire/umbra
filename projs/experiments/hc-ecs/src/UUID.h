#pragma once
#include "fmt/printf.h"
#include <string>

/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

inline uint8_t byteFromHex(char c1, char c2) {
    static const uint_fast8_t lookup[256] = {
        ['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3, ['4'] = 0x4, ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7,
        ['8'] = 0x8, ['9'] = 0x9, ['A'] = 0xA, ['B'] = 0xB, ['C'] = 0xC, ['D'] = 0xD, ['E'] = 0xE, ['F'] = 0xF,
    };

    return lookup[c1] << 4 | lookup[c2];
}

namespace sonic {
    /**
     * Globally unique, per-runtime, 128-bit, hashable ID.
     * String representable in the form XYXYXYXY-XYXY-XYXY-XYXY-XYXYXYXYXYXY where XY is a hex byte.
     */
    class UUID final {

        union UUIDData {
            uint64_t u64[2];
            uint32_t u32[4];
            uint8_t  u8[16];
        };

    public:
        // Create a new UUID that is not already in use.
        static UUID Generate();
        // Check whether the given string contains a parseable UUID
        static bool IsValidUUID(const char* string);

        inline UUID() { memset(&data, 0, sizeof(data)); }
        inline explicit UUID(uint64_t i0, uint64_t i1) { data.u64[0] = i0; data.u64[1] = i1; }
        inline explicit UUID(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) { data.u32[0] = i0; data.u32[1] = i1; data.u32[2] = i2; data.u32[3] = i3; }
        // Expects a valid UUID String value - use IsValidUUID to check
        inline explicit UUID(const std::string& str) : UUID(str.c_str()) {}
        // Expects a valid UUID String value - use IsValidUUID to check
        explicit UUID(const char* str);

        inline std::string ToString() const {
            return std::string(fmt::sprintf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x", data.u8[0], data.u8[1], data.u8[2], data.u8[3], data.u8[4], data.u8[5], data.u8[6], data.u8[7], data.u8[8], data.u8[9], data.u8[10], data.u8[11], data.u8[12], data.u8[13], data.u8[14], data.u8[15]));
        }

        inline bool IsValid() const { return data.u64[0] != 0 && data.u64[1] != 0; }
        inline void Clear() { memset(&data, 0, sizeof(data)); }

        bool operator==(UUID const& r) const { return data.u64[0] == r.data.u64[0] && data.u64[1] == r.data.u64[1]; }
        bool operator!=(UUID const& r) const { return data.u64[0] != r.data.u64[0] || data.u64[1] != r.data.u64[1]; }

    private:
        UUIDData data;
    };
}

template<>
struct std::hash<sonic::UUID> {
    std::size_t operator()(const sonic::UUID &s) const noexcept;
};
