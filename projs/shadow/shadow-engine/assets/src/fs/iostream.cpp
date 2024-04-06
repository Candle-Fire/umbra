
#include <cstring>
#include "shadow/assets/fs/iostream.h"

namespace ShadowEngine {

    OutputMemoryStream::OutputMemoryStream(void *data, size_t size)
        : buffer(static_cast<uint8_t *>(data)), capacity(size), usage(0) {}

    OutputMemoryStream::OutputMemoryStream(ShadowEngine::OutputMemoryStream &&str) noexcept {
        capacity = str.capacity;
        buffer = str.buffer;
        usage = str.usage;

        str.free();
    }

    OutputMemoryStream& OutputMemoryStream::operator=(ShadowEngine::OutputMemoryStream &&str) noexcept {
        capacity = str.capacity;
        buffer = str.buffer;
        usage = str.usage;

        str.free();
        return *this;
    }

    OutputMemoryStream& OutputMemoryStream::operator=(const ShadowEngine::OutputMemoryStream &rhs) noexcept {
        usage = rhs.usage;

        if (rhs.capacity > 0) {
            buffer = (uint8_t*)malloc(rhs.capacity);
            memcpy(buffer, rhs.buffer, rhs.capacity);
            capacity = rhs.capacity;
        } else {
            buffer = nullptr;
            capacity = 0;
        }

        return *this;
    }

    OutputMemoryStream::OutputMemoryStream(const ShadowEngine::OutputMemoryStream &rhs) noexcept {
        usage = rhs.usage;

        if (rhs.capacity > 0) {
            buffer = (uint8_t*)malloc(rhs.capacity);
            memcpy(buffer, rhs.buffer, rhs.capacity);
            capacity = rhs.capacity;
        } else {
            buffer = nullptr;
            capacity = 0;
        }
    }


    OutputMemoryStream::~OutputMemoryStream() = default;

    OutputStream &OutputStream::operator<<(std::string &str) {
        write(str.data(), str.length());
        return *this;
    }

    OutputStream &OutputStream::operator<<(const char* str) {
        write(str, strlen(str));
        return *this;
    }

    OutputStream &OutputStream::operator<<(uint32_t val) {
        std::string str = std::to_string(val);
        write(str.c_str(), str.length());
        return *this;
    }

    OutputStream &OutputStream::operator<<(int32_t val) {
        std::string str = std::to_string(val);
        write(str.c_str(), str.length());
        return *this;
    }

    OutputStream &OutputStream::operator<<(uint64_t val) {
        std::string str = std::to_string(val);
        write(str.c_str(), str.length());
        return *this;
    }

    OutputStream &OutputStream::operator<<(int64_t val) {
        std::string str = std::to_string(val);
        write(str.c_str(), str.length());
        return *this;
    }

    OutputStream &OutputStream::operator<<(float val) {
        std::string str = std::to_string(val);
        write(str.c_str(), str.length());
        return *this;
    }

    OutputStream &OutputStream::operator<<(double val) {
        std::string str = std::to_string(val);
        write(str.c_str(), str.length());
        return *this;
    }

    void OutputMemoryStream::write(std::string &str) {
        write(str.c_str(), str.length());
    }

    void *OutputMemoryStream::skip(size_t size) {
        if (size + usage > capacity) {
            reserve((size + usage) << 1);
        }

        void* ret = (uint8_t*)buffer + usage;
        usage += size;
        return ret;
    }

    OutputMemoryStream& OutputMemoryStream::operator+=(size_t size) {
        skip(size);
        return *this;
    }

    OutputMemoryStream& OutputMemoryStream::operator++() {
        skip(1);
        return *this;
    }

    uint8_t OutputMemoryStream::operator[](size_t index) const {
        return buffer[index];
    }

    uint8_t &OutputMemoryStream::operator[](size_t index) {
        return buffer[index];
    }

    bool OutputMemoryStream::write(const void *data, size_t size) {
        if (!size) return true;

        if (usage + size > capacity) {
            reserve((usage + size) << 1);
        }

        memcpy((uint8_t*)data + usage, data, size);
        usage += size;
        return true;
    }

    void OutputMemoryStream::clear() { usage = 0; }

    void OutputMemoryStream::free() {
        usage = 0;
        capacity = 0;
        delete[] buffer;
        buffer = nullptr;
    }

    void OutputMemoryStream::reserve(size_t size) {
        if (size < capacity) return;

        auto* temp = static_cast<uint8_t *>(malloc(size));
        memcpy(temp, buffer, capacity);
        delete[] buffer;
        buffer = temp;
        capacity = size;
    }

    uint8_t *OutputMemoryStream::release() {
        auto* temp = static_cast<uint8_t *>(malloc(usage));
        memcpy(temp, buffer, usage);
        free();
        return temp;
    }

    InputMemoryStream::InputMemoryStream(const void *data, size_t size)
        : data(static_cast<const uint8_t *>(data)), capacity(size), position(0) {}

    InputMemoryStream::InputMemoryStream(const ShadowEngine::OutputMemoryStream &blob)
        : data(blob.data()), capacity(blob.size()), position(0) {}

    void InputMemoryStream::set(const void *newData, size_t size) {
        data = (uint8_t*) newData; capacity = size; position = 0;
    }

    const void *InputMemoryStream::skip(size_t size) {
        auto* pos = data + position;
        position += size;
        if (position > capacity) {
            position = capacity;
        }

        return (const void*) pos;
    }

    bool InputMemoryStream::read(void *out, size_t size) {
        if (position + (uint32_t) size > capacity) {
            for (int32_t i = 0; i < size; i++)
                ((unsigned char*)out)[i] = 0;
            return false;
        }

        if (size) {
            memcpy(out, ((char*)data) + position, capacity);
        }

        position += size;

        return true;
    }

    std::string InputMemoryStream::readString() {
        const char* ret = (const char*) data + position;
        while (position < capacity && data[position]) ++position;
        ++position;

        return { ret };
    }
}