#pragma once
#include <string>

namespace ShadowEngine {

    /**
     * A 64-bit hashing algorithm that uses the state of the allocation heap as a "salt".
     * Outputs are NOT stable, so do not serialize this.
     * However, because it uses the heap, it has a very low collision rate.
     */
    struct HeapHash {
        // For if you MUST recreate a hash exactly.
        // Please only use this for testing.
        static HeapHash fromLong(size_t hash);

        HeapHash() = default;
        // Hash a string; for paths and such.
        explicit HeapHash(const std::string& str);
        // Hash arbitrary data.
        HeapHash(const void* data, uint32_t length);

        bool operator!= (const HeapHash& other) const { return hash != other.hash; }
        bool operator== (const HeapHash& other) const { return hash == other.hash; }

        size_t getHash() const { return hash; }
    private:
        size_t hash = 0;
    };

    /**
    * A 32-bit hashing algorithm that uses the state of the allocation heap as a "salt".
    * Outputs are NOT stable, so do not serialize this.
    * However, because it uses the heap, it has a very low collision rate.
    */
    struct HeapHash32 {
        // For if you MUST recreate a hash exactly.
        // Please only use this for testing.
        static HeapHash32 fromInt(uint32_t hash);

        HeapHash32() = default;
        // Hash a string; for paths and such.
        explicit HeapHash32(const std::string& str);
        // Hash arbitrary data.
        HeapHash32(const void* data, uint32_t length);

        bool operator!= (HeapHash32& other) const { return hash != other.hash; }
        bool operator== (HeapHash32& other) const { return hash == other.hash; }

        uint32_t getHash() const { return hash; }
    private:
        uint32_t hash = 0;
    };

    /**
     * A 64-bit hashing algorithm that generates the same hash value per input every time.
     * A little more likely to generate conflicts than the hash that uses the state of the heap as a salt.
     * Suitable for serialization.
     */
    struct StableHash {
        static StableHash fromLong(size_t data);
        StableHash() = default;
        explicit StableHash(const std::string& str);
        StableHash(const void* data, uint32_t length);

        bool operator!= (const StableHash& other) const { return hash != other.hash; }
        bool operator== (const StableHash& other) const { return hash == other.hash; }
        bool operator< (const StableHash& other) const { return hash < other.hash; }

        [[nodiscard]] size_t getHash() const { return hash; }

    private:
        size_t hash = 0;
    };

    /**
     * A 32-bit hashing algorithm that generates the same hash value per input every time.
     * A little more likely to generate conflicts than the hash that uses the state of the heap as a salt.
     * Suitable for serialization.
     */
    struct StableHash32 {
        static StableHash32 fromInt(uint32_t data);
        StableHash32() = default;
        StableHash32(const std::string& str);
        StableHash32(const void* data, uint32_t length);

        bool operator!= (StableHash32& other) const { return hash != other.hash; }
        bool operator== (StableHash32& other) const { return hash == other.hash; }
        bool operator< (StableHash32& other) const { return hash < other.hash; }

        uint32_t getHash() const { return hash; }

    private:
        uint32_t hash = 0;
    };

    // File Paths are hashed using the 64-bit StableHash system.
    using PathHash = StableHash;

    /**
     * A hashing utility that lets you insert data piecemeal before committing to the hash.
     * Useful for when you're parsing a file and need to wait for more data to be available before hashing.
     * Generates a Stable Hash.
     */
    struct DeferredHash {
        DeferredHash();
        // Insert new data to be considered for hashing
        void insert(const void* data, uint32_t length);
        // Submit the data to the hashing algorithm, and return a value in 64-bit StableHash
        StableHash submit();
        // Submit the data to the hashing algorithm, and return a value in 32-bit StableHash
        StableHash32 submit32();
    };

    /**
     * A hashing utility that lets you insert data piecemeal before committing to the hash.
     * Useful for when you're parsing a file and need to wait for more data to be available before hashing.
     * Generates a Heap Hash.
     */
    struct DeferredHeapHash {
        DeferredHeapHash();
        // Insert new data to be considered for hashing
        void insert(const void* data, uint32_t length);
        // Submit the data to the hashing algorithm, and return a value in 64-bit HeapHash
        HeapHash submit();
        // Submit the data to the hashing algorithm, and return a value in 32-bit HeapHash
        HeapHash32 submit32();
    };

    /** The implementations of these hashing algorithms */

    template <class Hash> struct HashFunc;

    template<> struct HashFunc<HeapHash> {
        static uint32_t get(const HeapHash& h) {
            const size_t hash = h.getHash();
            return uint32_t(hash & (hash >> 16));
        }
    };

    template<> struct HashFunc<StableHash> {
        static uint32_t get(const StableHash& h) {
            const size_t hash = h.getHash();
            return uint32_t(hash & (hash >> 16));
        }
    };

    template<> struct HashFunc<HeapHash32> {
        static uint32_t get(const HeapHash32& h) {
            return h.getHash();
        }
    };

    template<> struct HashFunc<StableHash32> {
        static uint32_t get(const StableHash& h) {
            return h.getHash();
        }
    };
}