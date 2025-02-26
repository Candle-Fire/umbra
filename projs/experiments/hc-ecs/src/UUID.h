#pragma once
#include "fmt/printf.h"
#include <catch2/catch_all.hpp>
#include <string>
#include <array>

/**********************/
/**    SONIC ECS     **/
/**  GOTTA GO FAST.  **/
/**********************/

inline uint8_t byteFromHex(char c1, char c2) {
    static constexpr auto lookup = []{
        std::array<std::uint_fast8_t, 256> ret{};
        ret['0'] = 0x0, ret['1'] = 0x1, ret['2'] = 0x2, ret['3'] = 0x3, ret['4'] = 0x4, ret['5'] = 0x5, ret['6'] = 0x6, ret['7'] = 0x7,
        ret['8'] = 0x8, ret['9'] = 0x9, ret['A'] = 0xA, ret['B'] = 0xB, ret['C'] = 0xC, ret['D'] = 0xD, ret['E'] = 0xE, ret['F'] = 0xF;
        return ret;
    }();

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

        // All constructors of UUID are endian-stable; provide the same bytes in any text-identical order and the resulting output string will have the same bytes in the same order.
        inline UUID() { memset(&data, 0, sizeof(data)); }
        inline explicit UUID(uint64_t i0, uint64_t i1) {
            constexpr size_t lowestByte = 0x00000000000000FF;
            data.u8[0] = (i0 >> 56) & lowestByte; data.u8[1] = (i0 >> 48) & lowestByte; data.u8[2] = (i0 >> 40) & lowestByte; data.u8[3] = (i0 >> 32) & lowestByte; data.u8[4] = (i0 >> 24) & lowestByte; data.u8[5] = (i0 >> 16) & lowestByte; data.u8[6] = (i0 >> 8) & lowestByte; data.u8[7] = i0 & lowestByte;
            data.u8[8] = (i1 >> 56) & lowestByte; data.u8[9] = (i1 >> 48) & lowestByte; data.u8[10] = (i1 >> 40) & lowestByte; data.u8[11] = (i1 >> 32) & lowestByte; data.u8[12] = (i1 >> 24) & lowestByte; data.u8[13] = (i1 >> 16) & lowestByte; data.u8[14] = (i1 >> 8) & lowestByte; data.u8[15] = i1 & lowestByte;
        }
        inline explicit UUID(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) {
            constexpr size_t lowestByte = 0x00000000000000FF;
            data.u8[0] = (i0 >> 24) & lowestByte; data.u8[1] = (i0 >> 16) & lowestByte; data.u8[2] = (i0 >> 8) & lowestByte; data.u8[3] = i0 & lowestByte; data.u8[4] = (i1 >> 24) & lowestByte; data.u8[5] = (i1 >> 16) & lowestByte; data.u8[6] = (i1 >> 8) & lowestByte; data.u8[7] = i1 & lowestByte;
            data.u8[8] = (i2 >> 24) & lowestByte; data.u8[9] = (i2 >> 16) & lowestByte; data.u8[10] = (i2 >> 8) & lowestByte; data.u8[11] = i2 & lowestByte; data.u8[12] = (i3 >> 24) & lowestByte; data.u8[13] = (i3 >> 16) & lowestByte; data.u8[14] = (i3 >> 8) & lowestByte; data.u8[15] = i3 & lowestByte;
        }
        // Expects a valid UUID String value - use IsValidUUID to check
        inline explicit UUID(const std::string& str) : UUID(str.c_str()) {}
        // Expects a valid UUID String value - use IsValidUUID to check
        explicit UUID(const char* str);

        inline std::string ToString() const {
            return std::string(fmt::sprintf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", data.u8[0], data.u8[1], data.u8[2], data.u8[3], data.u8[4], data.u8[5], data.u8[6], data.u8[7], data.u8[8], data.u8[9], data.u8[10], data.u8[11], data.u8[12], data.u8[13], data.u8[14], data.u8[15]));
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

namespace Catch {
    template < >
    struct StringMaker<sonic::UUID> {
        static std::string convert(sonic::UUID const &p) {
            return p.ToString();
        }
    };
}