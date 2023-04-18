#pragma once
#include <cstdint>
#include <cstring>
#include <string>

// ShadowEntity temporary namespace
namespace SE {

    /**
     * Universally Unique ID.
     * 128 Bits.
     *
     * Unique per runtime only - the suitability for serialization is undetermined.
     */

    class UUID {

        /**
         * Data storage; 128 bits.
         * 2 x 64 bit
         * 4 x 32 bit
         * 16 x 8 bit
         */

        union Data {
            uint64_t u64[2];
            uint32_t u32[4];
            uint8_t u8[16];
        };

    public:
        // Create a new, unused, UUID.
        static UUID Generate();
        // Check whether the UUID is correctly formed.
        static bool IsValidStr(char const* str);

        // Create an empty UUID.
        inline UUID() { std::memset(&data.u8, 0, 16); }
        // Create a UUID based on the given values.
        inline UUID(uint64_t i0, uint64_t i1) { data.u64[0] = i0; data.u64[1] = i1; }
        inline UUID(uint32_t i0, uint32_t i1, uint32_t i2, uint32_t i3) { data.u32[0] = i0; data.u32[1] = i1; data.u32[2] = i2; data.u32[3] = i3; }
        inline explicit UUID(std::string const& str) : UUID(str.c_str()) {}
        // Create a UUID from the given format.
        explicit UUID(char const* str);

        // Check whether the UUID is nonzero.
        inline bool IsValid() const { return data.u64[0] != 0 && data.u64[1] != 0; }
        // Set the UUID to zero.
        inline void Clear() { std::memset(&data.u8, 0, 16); }

        // Get a section of the UUID's data as the given bit width.
        inline uint8_t GetU8(size_t idx) const { return data.u8[idx]; }
        inline uint32_t GetU32(size_t idx) const { return data.u32[idx]; }
        inline uint64_t GetU64(size_t idx) const { return data.u64[idx]; }

        // Check whether this and a given UUID are in/equal.
        __inline bool operator==(UUID const& other) const { return data.u64[0] == other.data.u64[0] && data.u64[1] == other.data.u64[1]; }
        __inline bool operator!=(UUID const& other) const { return !(*this == other); }

    private:

        Data data {};
    };
}