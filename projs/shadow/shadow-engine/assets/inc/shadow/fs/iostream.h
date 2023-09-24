#pragma once
#include <string>

namespace ShadowEngine {
    // A custom OutputStream that can be implemented to output to any arbitrary data structure.
    // The idea is that it can write to a file, or into memory, or into a temporary buffer that is copied to both.
    // As opposed to the hardcoded streams that exist in C++, which have a single purpose for their entire lifetime.
    struct OutputStream {
        virtual bool write(const void* data, size_t size) = 0;

        OutputStream& operator<< (std::string& str);
        OutputStream& operator<< (const char* str);
        OutputStream& operator<< (size_t val);
        OutputStream& operator<< (int64_t val);
        OutputStream& operator<< (uint32_t val);
        OutputStream& operator<< (int32_t val);
        OutputStream& operator<< (float val);
        OutputStream& operator<< (double val);
        template <class T> bool write(const T& val);
    };

    // A custom InputStream that can be implemented to read from any arbitrary data structure.
    // The idea is that it can read from a file, or from memory, or from a temporary buffer that is merged from both.
    // As opposed to the hardcoded streams that exist in C++, which have a single purpose for their entire lifetime.
    struct InputStream {
        virtual bool read(void* buffer, size_t size) = 0;
        virtual const void* getBuffer() const = 0;
        virtual size_t size() const = 0;

        template <class T> void read(T& val) { read(&val, sizeof(T)); }
        template <class T> T read();
    };

    // A custom OutputStream that writes to memory.
    struct OutputMemoryStream final : OutputStream {

        OutputMemoryStream() : buffer(nullptr), capacity(0), usage(0) {};
        OutputMemoryStream(void* data, size_t size);
        OutputMemoryStream(OutputMemoryStream&& str) noexcept;
        OutputMemoryStream(const OutputMemoryStream& rhs) noexcept;
        ~OutputMemoryStream();

        OutputMemoryStream& operator= (const OutputMemoryStream& rhs) noexcept;
        OutputMemoryStream& operator= (OutputMemoryStream&& rhs) noexcept;

        uint8_t operator[] (size_t index) const;
        uint8_t& operator[] (size_t index);
        OutputMemoryStream& operator+= (size_t index);
        OutputMemoryStream& operator++ ();

        bool write(const void* data, size_t size) override;

        uint8_t* release();
        void resize(size_t size);
        void reserve(size_t size);
        const uint8_t* data() const { return buffer; };
        uint8_t* dataMut() { return buffer; };
        size_t size() const { return usage; };
        void clear();
        void* skip(size_t size);
        bool empty() const { return usage == 0; };
        void free();

        void write(std::string& str);
        template <class T> void write(const T& val);

    private:
        uint8_t* buffer;
        size_t capacity;
        size_t usage;
    };

    template <class T> void OutputMemoryStream::write(const T& val){
        write(&val, sizeof(T));
    }


    template <> inline void OutputMemoryStream::write<bool>(const bool& val) {
        uint8_t v = val;
        write(&v, sizeof(v));
    }

    // A custom InputStream that writes from memory.
    struct InputMemoryStream final : InputStream {
        InputMemoryStream(const void* data, size_t size);
        explicit InputMemoryStream(const OutputMemoryStream& blob);

        void set(const void* data, size_t size);
        bool read(void* data, size_t size) override;
        std::string readString();
        const void* skip(size_t size);
        const void* getData() const { return data; };
        const void* getBuffer() const override { return data; };
        size_t size() const override { return capacity; };
        size_t pos() const { return position; };
        void setPos(size_t pos) { position = pos; };
        void restart() { position = 0; };
        uint8_t readChar() { position++; return data[position-1]; };

        template<class T>
        T getAs() const {
            static_assert(position + sizeof(T) < capacity);
            return *(T*)(data + position);
        }

        using InputStream::read;

    private:
        const uint8_t* data;
        size_t capacity;
        size_t position;
    };

    template <class T>
    T InputStream::read() {
        T v;
        read(&v, sizeof(T));
        return v;
    }

    template<> inline bool InputStream::read<bool>() {
        uint8_t v;
        read(&v, sizeof(bool));
        return v;
    }

    template <class T>
    bool OutputStream::write(const T &val) {
        return write(&val, sizeof(T));
    }
}